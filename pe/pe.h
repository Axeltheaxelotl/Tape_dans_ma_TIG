#ifndef PE_H
#define PE_H

#include "../includes/woody.h"
#include <sys/stat.h>

// Les trucs magiques de Windows
#define IMAGE_DOS_SIGNATURE         0x5A4D
#define IMAGE_NT_SIGNATURE          0x00004550
#define IMAGE_FILE_MACHINE_I386     0x014C
#define IMAGE_FILE_MACHINE_AMD64    0x8664
#define IMAGE_SCN_CNT_CODE          0x00000020
#define IMAGE_SCN_MEM_EXECUTE       0x20000000
#define IMAGE_SCN_MEM_READ          0x40000000
#define IMAGE_SCN_MEM_WRITE         0x80000000
#define IMAGE_SIZEOF_SHORT_NAME     8
#define IMAGE_NT_OPTIONAL_HDR32_MAGIC 0x10B
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC 0x20B
#define IMAGE_NUMBEROF_DIRECTORY_ENTRIES 16

// Le header DOS (vieux truc MS-DOS mais obligatoire)
typedef struct s_image_dos_header {
    uint16_t e_magic;       // "MZ" (0x5A4D)
    uint16_t e_cblp;
    uint16_t e_cp;
    uint16_t e_crlc;
    uint16_t e_cparhdr;
    uint16_t e_minalloc;
    uint16_t e_maxalloc;
    uint16_t e_ss;
    uint16_t e_sp;
    uint16_t e_csum;
    uint16_t e_ip;
    uint16_t e_cs;
    uint16_t e_lfarlc;
    uint16_t e_ovno;
    uint16_t e_res[4];
    uint16_t e_oemid;
    uint16_t e_oeminfo;
    uint16_t e_res2[10];
    int32_t  e_lfanew;      // Ou se trouve le vrai PE header
} t_image_dos_header;

// Le header COFF (dit l'archi et le nombre de sections)
typedef struct s_image_file_header {
    uint16_t Machine;                   // 0x014C = x86, 0x8664 = x64
    uint16_t NumberOfSections;          // Nombre de sections (.text, .data, etc.)
    uint32_t TimeDateStamp;             // Date de compile (on s'en tape)
    uint32_t PointerToSymbolTable;      // Toujours 0, obsolete
    uint32_t NumberOfSymbols;           // Toujours 0, obsolete
    uint16_t SizeOfOptionalHeader;      // Taille du optional header
    uint16_t Characteristics;           // Des flags chelou
} t_image_file_header;

typedef struct s_image_data_directory {
    uint32_t VirtualAddress;
    uint32_t Size;
} t_image_data_directory;

// Optional Header 32 bits (EN VRAI C'EST PAS OPTIONAL)
typedef struct s_image_optional_header32 {
    uint16_t Magic;                     // 0x10B pour PE32
    uint8_t  MajorLinkerVersion;        // Versions de linker et autre merde
    uint8_t  MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;       // RVA du point d'entree
    uint32_t BaseOfCode;                // RVA du code
    uint32_t BaseOfData;                // RVA des donnees
    uint32_t ImageBase;                 // Adresse de base en memoire
    uint32_t SectionAlignment;          // Alignement en memoire (genre 4KB)
    uint32_t FileAlignment;             // Alignement sur disque (genre 512 bytes)
    uint16_t MajorOperatingSystemVersion;
    uint16_t MinorOperatingSystemVersion;
    uint16_t MajorImageVersion;
    uint16_t MinorImageVersion;
    uint16_t MajorSubsystemVersion;
    uint16_t MinorSubsystemVersion;
    uint32_t Win32VersionValue;
    uint32_t SizeOfImage;               // Taille totale en memoire
    uint32_t SizeOfHeaders;             // Taille des headers
    uint32_t CheckSum;                  // Checksum, subsystem, etc.
    uint16_t Subsystem;
    uint16_t DllCharacteristics;
    uint32_t SizeOfStackReserve;
    uint32_t SizeOfStackCommit;
    uint32_t SizeOfHeapReserve;
    uint32_t SizeOfHeapCommit;
    uint32_t LoaderFlags;
    uint32_t NumberOfRvaAndSizes;
    t_image_data_directory DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} t_image_optional_header32;

// Optional Header 64 bits (meme bordel mais en 64 bits)
typedef struct s_image_optional_header64 {
    uint16_t Magic;                     // 0x20B pour PE32+
    uint8_t  MajorLinkerVersion;        // Versions
    uint8_t  MinorLinkerVersion;
    uint32_t SizeOfCode;
    uint32_t SizeOfInitializedData;
    uint32_t SizeOfUninitializedData;
    uint32_t AddressOfEntryPoint;       // RVA du point d'entree
    uint32_t BaseOfCode;                // RVA du code
    uint64_t ImageBase;                 // Adresse de base (64-bit cette fois)
    uint32_t SectionAlignment;          // Alignement memoire
    uint32_t FileAlignment;             // Alignement fichier
    uint16_t MajorOperatingSystemVersion;
    uint16_t MinorOperatingSystemVersion;
    uint16_t MajorImageVersion;
    uint16_t MinorImageVersion;
    uint16_t MajorSubsystemVersion;
    uint16_t MinorSubsystemVersion;
    uint32_t Win32VersionValue;
    uint32_t SizeOfImage;               // Taille totale
    uint32_t SizeOfHeaders;             // Taille headers
    uint32_t CheckSum;                  // Checksum, subsystem, etc.
    uint16_t Subsystem;
    uint16_t DllCharacteristics;
    uint64_t SizeOfStackReserve;
    uint64_t SizeOfStackCommit;
    uint64_t SizeOfHeapReserve;
    uint64_t SizeOfHeapCommit;
    uint32_t LoaderFlags;
    uint32_t NumberOfRvaAndSizes;
    t_image_data_directory DataDirectory[IMAGE_NUMBEROF_DIRECTORY_ENTRIES];
} t_image_optional_header64;

typedef struct s_image_nt_headers32 {
    uint32_t Signature;
    t_image_file_header FileHeader;
    t_image_optional_header32 OptionalHeader;
} t_image_nt_headers32;

typedef struct s_image_nt_headers64 {
    uint32_t Signature;
    t_image_file_header FileHeader;
    t_image_optional_header64 OptionalHeader;
} t_image_nt_headers64;

// Section header (une entree pour chaque section .text, .data, etc.)
typedef struct s_image_section_header {
    uint8_t  Name[IMAGE_SIZEOF_SHORT_NAME];    // Nom de la section (ex: ".text")
    uint32_t VirtualSize;                      // Taille en memoire
    uint32_t VirtualAddress;                   // RVA (adresse relative)
    uint32_t SizeOfRawData;                    // Taille sur disque
    uint32_t PointerToRawData;                 // Offset dans le fichier
    uint32_t PointerToRelocations;             // Relocations et autres merdes obsoletes
    uint32_t PointerToLinenumbers;
    uint16_t NumberOfRelocations;
    uint16_t NumberOfLinenumbers;
    uint32_t Characteristics;                  // Flags de la section
} t_image_section_header;

// Notre structure principale pour un fichier PE
typedef struct s_pe_file {
    void    *base_addr;                 // Debut du fichier mmap
    void    *end_addr;                  // Fin du fichier mmap
    void    *text_section;              // Pointeur vers .text
    size_t   text_size;                 // Taille de .text
    uint32_t text_rva;                  // RVA de .text
    uint32_t original_entry_point;      // Point d'entree original
    char     encryption_key[KEY_SIZE];  // Cle de chiffrement
    off_t    file_size;                 // Taille du fichier
    t_arch   arch_type;                 // 32 ou 64 bits
    int      file_fd;                   // Descripteur fichier
    int      is_key_provided;           // 1 si cle fournie
    int      is_compressed;             // 1 si compresse
    size_t   compressed_size;           // Taille compressée
    size_t   original_size;             // Taille originale
    t_image_dos_header       *dos_header;       // Header DOS
    t_image_nt_headers32     *nt_headers32;     // NT headers (32 ou 64 selon arch)
    t_image_nt_headers64     *nt_headers64;
    t_image_section_header   *section_headers;  // Tableau de sections
    uint16_t                  number_of_sections; // Nombre de sections
} t_pe_file;

#define PE_DECRYPTION_CODE_32 "TBD"
#define PE_DECRYPTION_CODE_64 "TBD"
#define PE_CODE_SIZE_32 128
#define PE_CODE_SIZE_64 128

// Fonctions pour gerer les PE
int     pe_parse(const char *filename, t_pe_file *pe);
void    pe_encrypt(t_pe_file *pe);
void    pe_inject(t_pe_file *pe, t_injection_payload *payload);
int     pe_is_valid(const void *base, size_t size);
void    pe_cleanup(t_pe_file *pe);

// Fonctions utilitaires
t_image_section_header *pe_find_section(t_pe_file *pe, const char *name);
uint32_t pe_align(uint32_t value, uint32_t alignment);
uint32_t pe_rva_to_offset(t_pe_file *pe, uint32_t rva);

#endif
