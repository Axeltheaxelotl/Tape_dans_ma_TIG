#include "pe.h"

/*
 * Générateur de PE minimaliste pour tester
 * Crée un vrai PE32 avec une section .text
 */

// Code machine simple qui fait exit(42) - x86 32-bit
unsigned char shellcode_32[] = {
    0xB8, 0x2A, 0x00, 0x00, 0x00,  // mov eax, 42
    0x50,                          // push eax
    0x6A, 0x00,                    // push 0
    0xFF, 0x15, 0x00, 0x20, 0x00, 0x00,  // call [ExitProcess]
    0xCC                           // int3 (au cas où)
};

// Code machine simple qui fait exit(42) - x86-64
unsigned char shellcode_64[] = {
    0x48, 0xC7, 0xC1, 0x2A, 0x00, 0x00, 0x00,  // mov rcx, 42
    0x48, 0x83, 0xEC, 0x28,                    // sub rsp, 0x28
    0xFF, 0x15, 0x02, 0x00, 0x00, 0x00,        // call [ExitProcess]
    0xEB, 0xFE,                                // jmp $
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  // adresse ExitProcess
};

typedef struct s_pe_generator {
    int is_64bit;
    size_t code_size;
    unsigned char *code;
} t_pe_generator;

void write_dos_header(FILE *f)
{
    t_image_dos_header dos = {0};
    
    dos.e_magic = IMAGE_DOS_SIGNATURE;  // "MZ"
    dos.e_cblp = 0x90;
    dos.e_cp = 0x03;
    dos.e_cparhdr = 0x04;
    dos.e_maxalloc = 0xFFFF;
    dos.e_sp = 0xB8;
    dos.e_lfarlc = 0x40;
    dos.e_lfanew = 0x80;  // Offset vers PE header
    
    fwrite(&dos, sizeof(dos), 1, f);
    
    // Remplir jusqu'à 0x80 avec le stub DOS
    unsigned char stub[0x80 - sizeof(dos)];
    memset(stub, 0, sizeof(stub));
    // Message "This program cannot be run in DOS mode"
    const char *msg = "This program cannot be run in DOS mode.\r\r\n$";
    memcpy(stub, msg, strlen(msg));
    fwrite(stub, sizeof(stub), 1, f);
}

void write_pe32_headers(FILE *f, t_pe_generator *gen)
{
    uint32_t pe_sig = IMAGE_NT_SIGNATURE;
    t_image_file_header file_header = {0};
    t_image_optional_header32 opt_header = {0};
    
    (void)gen;
    
    // Signature PE
    fwrite(&pe_sig, sizeof(pe_sig), 1, f);
    
    // File header
    file_header.Machine = IMAGE_FILE_MACHINE_I386;
    file_header.NumberOfSections = 1;  // Juste .text
    file_header.TimeDateStamp = 0x12345678;
    file_header.SizeOfOptionalHeader = sizeof(t_image_optional_header32);
    file_header.Characteristics = 0x010F;  // IMAGE_FILE_EXECUTABLE_IMAGE | etc.
    fwrite(&file_header, sizeof(file_header), 1, f);
    
    // Optional header
    opt_header.Magic = IMAGE_NT_OPTIONAL_HDR32_MAGIC;
    opt_header.MajorLinkerVersion = 14;
    opt_header.MinorLinkerVersion = 0;
    opt_header.SizeOfCode = 0x200;  // Aligné
    opt_header.AddressOfEntryPoint = 0x1000;  // RVA du code
    opt_header.BaseOfCode = 0x1000;
    opt_header.BaseOfData = 0x2000;
    opt_header.ImageBase = 0x00400000;
    opt_header.SectionAlignment = 0x1000;
    opt_header.FileAlignment = 0x200;
    opt_header.MajorOperatingSystemVersion = 5;
    opt_header.MinorOperatingSystemVersion = 1;
    opt_header.MajorSubsystemVersion = 5;
    opt_header.MinorSubsystemVersion = 1;
    opt_header.SizeOfImage = 0x3000;
    opt_header.SizeOfHeaders = 0x200;
    opt_header.Subsystem = 3;  // Console
    opt_header.SizeOfStackReserve = 0x100000;
    opt_header.SizeOfStackCommit = 0x1000;
    opt_header.SizeOfHeapReserve = 0x100000;
    opt_header.SizeOfHeapCommit = 0x1000;
    opt_header.NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
    
    fwrite(&opt_header, sizeof(opt_header), 1, f);
}

void write_pe64_headers(FILE *f, t_pe_generator *gen)
{
    uint32_t pe_sig = IMAGE_NT_SIGNATURE;
    t_image_file_header file_header = {0};
    t_image_optional_header64 opt_header = {0};
    
    (void)gen;
    
    // Signature PE
    fwrite(&pe_sig, sizeof(pe_sig), 1, f);
    
    // File header
    file_header.Machine = IMAGE_FILE_MACHINE_AMD64;
    file_header.NumberOfSections = 1;
    file_header.TimeDateStamp = 0x12345678;
    file_header.SizeOfOptionalHeader = sizeof(t_image_optional_header64);
    file_header.Characteristics = 0x0022;  // executable
    fwrite(&file_header, sizeof(file_header), 1, f);
    
    // Optional header
    opt_header.Magic = IMAGE_NT_OPTIONAL_HDR64_MAGIC;
    opt_header.MajorLinkerVersion = 14;
    opt_header.SizeOfCode = 0x200;
    opt_header.AddressOfEntryPoint = 0x1000;
    opt_header.BaseOfCode = 0x1000;
    opt_header.ImageBase = 0x0000000140000000ULL;
    opt_header.SectionAlignment = 0x1000;
    opt_header.FileAlignment = 0x200;
    opt_header.MajorOperatingSystemVersion = 6;
    opt_header.MajorSubsystemVersion = 6;
    opt_header.SizeOfImage = 0x3000;
    opt_header.SizeOfHeaders = 0x200;
    opt_header.Subsystem = 3;
    opt_header.SizeOfStackReserve = 0x100000;
    opt_header.SizeOfStackCommit = 0x1000;
    opt_header.SizeOfHeapReserve = 0x100000;
    opt_header.SizeOfHeapCommit = 0x1000;
    opt_header.NumberOfRvaAndSizes = IMAGE_NUMBEROF_DIRECTORY_ENTRIES;
    
    fwrite(&opt_header, sizeof(opt_header), 1, f);
}

void write_section_header(FILE *f, t_pe_generator *gen)
{
    t_image_section_header section = {0};
    
    // Nom de la section
    strncpy((char *)section.Name, ".text", IMAGE_SIZEOF_SHORT_NAME);
    
    section.VirtualSize = gen->code_size;
    section.VirtualAddress = 0x1000;
    section.SizeOfRawData = 0x200;  // Aligné sur FileAlignment
    section.PointerToRawData = 0x200;  // Après les headers
    section.Characteristics = IMAGE_SCN_CNT_CODE | 
                             IMAGE_SCN_MEM_EXECUTE | 
                             IMAGE_SCN_MEM_READ;
    
    fwrite(&section, sizeof(section), 1, f);
}

int generate_pe(const char *filename, t_pe_generator *gen)
{
    FILE *f;
    unsigned char padding[0x200];
    long current_pos;
    
    f = fopen(filename, "wb");
    if (!f)
    {
        perror("fopen");
        return (EXIT_FAILURE);
    }
    
    // 1. DOS header + stub
    write_dos_header(f);
    
    // 2. PE headers
    if (gen->is_64bit)
        write_pe64_headers(f, gen);
    else
        write_pe32_headers(f, gen);
    
    // 3. Section header
    write_section_header(f, gen);
    
    // 4. Padding jusqu'à 0x200 (début de la section)
    current_pos = ftell(f);
    if (current_pos < 0x200)
    {
        memset(padding, 0, sizeof(padding));
        fwrite(padding, 0x200 - current_pos, 1, f);
    }
    
    // 5. Écrire le code
    fwrite(gen->code, gen->code_size, 1, f);
    
    // 6. Padding pour aligner à 0x200
    current_pos = ftell(f);
    size_t section_end = 0x200 + 0x200;
    if (current_pos < (long)section_end)
    {
        memset(padding, 0, sizeof(padding));
        fwrite(padding, section_end - current_pos, 1, f);
    }
    
    fclose(f);
    
    printf("✅ PE généré: %s\n", filename);
    printf("   Architecture: %s\n", gen->is_64bit ? "64-bit" : "32-bit");
    printf("   Taille code: %zu octets\n", gen->code_size);
    printf("   Taille totale: %zu octets\n", section_end);
    
    return (EXIT_SUCCESS);
}

void print_usage(const char *prog)
{
    printf("Usage: %s [32|64] <output.exe>\n", prog);
    printf("  Génère un PE minimal pour tester le parser\n");
    printf("\nExemples:\n");
    printf("  %s 32 test32.exe    # Crée un PE 32-bit\n", prog);
    printf("  %s 64 test64.exe    # Crée un PE 64-bit\n", prog);
}

int main(int argc, char **argv)
{
    t_pe_generator gen = {0};
    
    if (argc != 3)
    {
        print_usage(argv[0]);
        return (EXIT_FAILURE);
    }
    
    // Parser les arguments
    if (strcmp(argv[1], "32") == 0)
    {
        gen.is_64bit = 0;
        gen.code = shellcode_32;
        gen.code_size = sizeof(shellcode_32);
    }
    else if (strcmp(argv[1], "64") == 0)
    {
        gen.is_64bit = 1;
        gen.code = shellcode_64;
        gen.code_size = sizeof(shellcode_64);
    }
    else
    {
        printf("❌ Architecture invalide: %s (doit être 32 ou 64)\n", argv[1]);
        return (EXIT_FAILURE);
    }
    
    printf("\n🔨 Génération d'un PE %s-bit...\n", gen.is_64bit ? "64" : "32");
    
    if (generate_pe(argv[2], &gen) != EXIT_SUCCESS)
    {
        printf("❌ Erreur lors de la génération\n");
        return (EXIT_FAILURE);
    }
    
    printf("\n✨ Vous pouvez maintenant tester avec:\n");
    printf("   ./pe_test %s\n\n", argv[2]);
    
    return (EXIT_SUCCESS);
}
