#ifndef WOODY_H
#define WOODY_H


#include <libft.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <elf.h>

//types personnaluses
typedef uint16_t t_arch;
typedef uint16_t t_endian;
//t_arch et t_endian: type pour stocker l architecture (32 ou 64 bites) et l endianess (little ou big)

//valeur de elf.h pour savoir comment lire les entiers dans l ELF
#define LENDIAN ELFDATA2LSB
#define BENDIAN ELFDATA2MSB



//Definition des codes de decryptage
#define DECRYPTION_CODE "\x50\x56\x57\x52\x51\x41\x50\x41\x51\xeb\x55\x5e\xb8\x01\x00\x00\x00\xbf\x01\x00\x00\x00\xba\x0e\x00\x00\x00\x0f\x05\xb9\x2b\x00\x00\x00\xbe\x04\x00\x00\x00\x48\x8d\x15\x09\x00\x00\x00\x4d\x31\xc0\x48\x31\xc0\xeb\x3d\x5f\x42\x8a\x04\x07\x00\xe0\x30\x02\x48\xff\xc2\x49\xff\xc0\x49\x39\xf0\x75\x06\x80\xc4\x2a\x4d\x31\xc0\xe2\xe5\x41\x59\x41\x58\x59\x5a\x5f\x5e\x58\xe9\x26\x00\x00\x00\xe8\xa6\xff\xff\xff\x2e\x2e\x2e\x2e\x57\x4f\x4f\x44\x59\x2e\x2e\x2e\x2e\x0a\xe8\xbe\xff\xff\xff" //64bites
#define DECRYPTION_CODE_32 "\x50\x56\x57\x52\x51\x53\xeb\x50\x5e\x89\xf1\xb8\x04\x00\x00\x00\xbb\x01\x00\x00\x00\xba\x0e\x00\x00\x00\xcd\x80\xb9\x2b\x00\x00\x00\xbe\x04\x00\x00\x00\xe8\x00\x00\x00\x00\x5a\x81\xc2\x0b\x0b\x0b\x0b\x31\xdb\x31\xc0\xeb\x33\x5f\x8a\x04\x1f\x00\xe0\x30\x02\x42\x43\x39\xf3\x75\x05\x80\xc4\x2a\x31\xdb\xe2\xec\x5b\x59\x5a\x5f\x5e\x58\xe9\xd2\xff\xff\xff\xe8\xab\xff\xff\xff\x2e\x2e\x2e\x2e\x57\x4f\x4f\x44\x59\x2e\x2e\x2e\x2e\x0a\xe8\xc8\xff\xff\xff" //32bites
#define CODE_SIZE sizeof(DECRYPTION_CODE) -1 + KEY_SIZE
#define CODE_SIZE_32 sizeof(DECRYPTION_CODE_32) -1 + KEY_SIZE
#define KEY_SIZE 32
/*
    DECRYPTION_CODE: code assembleur injecte dans l elf pour dechiffrer le programme a l execution

    CODE_SIZE: taille du code injecte + taille de la cle de chiffrement (KEY_SIZE 32)

    deux versions: 64 bits (DECRIPTION_CODE) et l autre 32 bites
*/

//erreur
#define ERROR_ERRNO 0
#define ERROR_PH_TRUNC 1
#define ERROR_TEXT_TRUNC 2
#define ERROR_ARGS 3


// Structure principale représentant un fichier ELF ouvert et mappé en mémoire
typedef struct s_elf_file
{
    void *base_addr;      // début du fichier mmap (start of mapped file)
    void *end_addr;       // fin du fichier mmap (end of mapped file)
    void *section_sex;   // segment .text
    void *note_section;   // segment .note
    char taille_key[KEY_SIZE]; // clé de chiffrement (encryption key)
    off_t file_size;      // taille du fichier (file size)
    t_arch arch_type;     // 32 ou 64 bits (architecture type)
    t_endian endian_type; // little ou big (endianness)
    int file_fd;          // descripteur de fichier (file descriptor)
} t_elf_file; //t file
// Cette structure représente un ELF ouvert et mappé en mémoire, avec infos d'archi et clé de chiffrement.


// Structure contenant le code à injecter et ses métadonnées
typedef struct s_injection_payload
{
    char *payload_code;         // code à injecter (injected code)
    size_t payload_size;        // taille du code (code size)
    uint16_t offset_text_size;  // offset pour la taille du segment .text
    uint16_t offset_key_size;   // offset pour la taille de la clé
    uint16_t offset_text;       // offset du segment .text
    uint16_t offset_key;        // offset de la clé
    uint16_t offset_jump;       // offset du saut (jump)
} t_injection_payload; //t_payload
// Utilisé pour patcher le code de décryptage avec les bonnes adresses et tailles.


typedef struct s_elf_segments
{
    void        *base_ptr;         // base pointer of the mapped ELF
    size_t      total_size;        // total size of the mapped ELF
    Elf64_Phdr  *data_segment;     // premier segment LOAD 64-bit (first LOAD segment)
    Elf64_Phdr  *last_segment;     // dernier segment LOAD 64-bit (last LOAD segment)
    Elf32_Phdr  *data_segment_32;  // premier segment LOAD 32-bit
    Elf32_Phdr  *last_segment_32;  // dernier segment LOAD 32-bit
} t_elf_segments; // t_woody
// Sert à localiser les segments où injecter le code dans l'ELF (used to locate LOAD segments for injection).

// Utiliser s_elf_segments pour les pointeurs sur la structure, pas le typedef t_elf_segments

extern void encryptitation(void *data, uint32_t data_len, void *text, uint32_t len_text);
void encryptitation_code(t_elf_file *file);


                         
//gestion de l endianess
uint16_t get_uint16(uint16_t bite, t_endian endian);
int32_t get_int32(int32_t bite, t_endian endian);
uint32_t get_uint32(uint32_t bite, t_endian endian);
uint64_t get_uint64(uint64_t bite, t_endian endian);
//fonction pour lire correcrtement selon le type d endianess de l ELF



//manipulation des segments ELF
Elf64_Phdr *segment(t_elf_file *file, int (*f)(Elf64_Phdr *));
Elf64_Phdr *last_load_segment(t_elf_file *file);
Elf32_Phdr *segment_32(t_elf_file *file);
Elf32_Phdr *seg_get32(t_elf_file *file, int (*filt)(Elf32_Phdr *));
Elf32_Phdr *get_last_load_segment_32(t_elf_file *file);
//cherche des segment specifique .text .data dans l ELF 32 ou 64
int is_text(Elf64_Phdr *phdr);
int is_data(Elf64_Phdr *phdr);
int is_text_32(Elf32_Phdr *phdr);
int is_data_32(Elf32_Phdr *phdr);
//fonctions filtre pour savoir si un segment est .text ou .data


//injection
void injectitation(t_elf_file *file, t_injection_payload *payload);
//injecte le playload (code + cle) dans l EL

//verification des erreurs
int le_paaarsing(const char *filename, t_elf_file *file);
void error_w(t_elf_file *file, t_injection_payload *payload, t_elf_segments *segments, int code);


#endif