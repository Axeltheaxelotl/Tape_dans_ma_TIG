#include "pe.h"
#include <fcntl.h>
#include <unistd.h>

// Stubs assembleur pré-compilés (binaires bruts)
extern unsigned char decrypt_win32_bin[];
extern unsigned char decrypt_win64_bin[];
extern unsigned int decrypt_win32_bin_len;
extern unsigned int decrypt_win64_bin_len;

// Offsets des données à patcher dans les stubs
#define STUB_OFFSET_TEXT_START_32      156  // Offset de text_start dans stub 32-bit
#define STUB_OFFSET_TEXT_SIZE_32       160  // Offset de text_size
#define STUB_OFFSET_ORIGINAL_EP_32     164  // Offset de original_entry_point
#define STUB_OFFSET_KEY_32             168  // Offset de key_data

#define STUB_OFFSET_TEXT_START_64      216  // Offset de text_start dans stub 64-bit
#define STUB_OFFSET_TEXT_SIZE_64       224  // Offset de text_size
#define STUB_OFFSET_ORIGINAL_EP_64     232  // Offset de original_entry_point
#define STUB_OFFSET_KEY_64             240  // Offset de key_data

/*
 * Charge le stub assembleur approprié
 */
static unsigned char *load_stub(t_pe_file *pe, size_t *stub_size)
{
    int fd;
    unsigned char *stub;
    ssize_t bytes_read;
    const char *stub_path;
    
    // Choisir le bon stub selon l'architecture
    if (pe->arch_type == ELFCLASS32)
    {
        stub_path = "asssrc/decrypt_win32.bin";
        *stub_size = 512; // Taille max estimée
    }
    else
    {
        stub_path = "asssrc/decrypt_win64.bin";
        *stub_size = 512;
    }
    
    // Ouvrir le fichier stub
    fd = open(stub_path, O_RDONLY);
    if (fd < 0)
    {
        ft_printf("Error: Cannot open stub file %s\n", stub_path);
        return NULL;
    }
    
    // Allouer et lire
    stub = ft_calloc(1, *stub_size);
    if (!stub)
    {
        close(fd);
        return NULL;
    }
    
    bytes_read = read(fd, stub, *stub_size);
    close(fd);
    
    if (bytes_read <= 0)
    {
        free(stub);
        return NULL;
    }
    
    *stub_size = bytes_read;
    return stub;
}

/*
 * Patche le stub avec les bonnes valeurs (RVAs, clé, etc.)
 */
static void patch_stub(unsigned char *stub, t_pe_file *pe, uint32_t stub_rva)
{
    uint32_t text_rva;
    uint32_t text_size;
    uint32_t original_ep;
    uint32_t text_offset_in_memory;
    
    (void)stub_rva; // Pas utilisé pour l'instant
    
    // Récupérer les infos selon l'architecture
    if (pe->arch_type == ELFCLASS32)
    {
        text_rva = pe->text_rva;
        text_size = pe->text_size;
        original_ep = pe->original_entry_point;
        text_offset_in_memory = pe->nt_headers32->OptionalHeader.ImageBase + text_rva;
        
        // Patcher le stub 32-bit
        *(uint32_t *)(stub + STUB_OFFSET_TEXT_START_32) = text_offset_in_memory;
        *(uint32_t *)(stub + STUB_OFFSET_TEXT_SIZE_32) = text_size;
        *(uint32_t *)(stub + STUB_OFFSET_ORIGINAL_EP_32) = 
            pe->nt_headers32->OptionalHeader.ImageBase + original_ep;
        ft_memcpy(stub + STUB_OFFSET_KEY_32, pe->encryption_key, KEY_SIZE);
    }
    else
    {
        text_rva = pe->text_rva;
        text_size = pe->text_size;
        original_ep = pe->original_entry_point;
        text_offset_in_memory = pe->nt_headers64->OptionalHeader.ImageBase + text_rva;
        
        // Patcher le stub 64-bit
        *(uint64_t *)(stub + STUB_OFFSET_TEXT_START_64) = text_offset_in_memory;
        *(uint64_t *)(stub + STUB_OFFSET_TEXT_SIZE_64) = text_size;
        *(uint64_t *)(stub + STUB_OFFSET_ORIGINAL_EP_64) = 
            pe->nt_headers64->OptionalHeader.ImageBase + original_ep;
        ft_memcpy(stub + STUB_OFFSET_KEY_64, pe->encryption_key, KEY_SIZE);
    }
}

/*
 * Crée une nouvelle section .woody dans le PE
 */
static void create_woody_section(t_pe_file *pe, size_t payload_size, t_image_section_header *new_section)
{
    t_image_section_header *last_section;
    uint32_t section_alignment;
    uint32_t file_alignment;
    
    last_section = &pe->section_headers[pe->number_of_sections - 1];
    
    if (pe->arch_type == ELFCLASS32)
    {
        section_alignment = pe->nt_headers32->OptionalHeader.SectionAlignment;
        file_alignment = pe->nt_headers32->OptionalHeader.FileAlignment;
    }
    else
    {
        section_alignment = pe->nt_headers64->OptionalHeader.SectionAlignment;
        file_alignment = pe->nt_headers64->OptionalHeader.FileAlignment;
    }
    
    ft_bzero(new_section, sizeof(t_image_section_header));
    ft_memcpy(new_section->Name, ".woody", 6);
    
    new_section->VirtualSize = payload_size;
    new_section->SizeOfRawData = pe_align(payload_size, file_alignment);
    
    new_section->VirtualAddress = pe_align(
        last_section->VirtualAddress + last_section->VirtualSize,
        section_alignment
    );
    
    new_section->PointerToRawData = pe_align(
        last_section->PointerToRawData + last_section->SizeOfRawData,
        file_alignment
    );
    
    new_section->Characteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | 
                                   IMAGE_SCN_MEM_READ | IMAGE_SCN_MEM_WRITE;
}

/*
 * Écrit le nouveau fichier PE avec la section injectée
 */
static int write_woody_pe(t_pe_file *pe, t_image_section_header *new_section, 
                          unsigned char *stub, size_t stub_size)
{
    int fd_out;
    size_t sections_table_offset;
    unsigned char *padding;
    size_t padding_size;
    
    (void)stub_size; // Utilisé implicitement via new_section->SizeOfRawData
    
    // Créer le fichier woody
    fd_out = open("woody", O_WRONLY | O_CREAT | O_TRUNC, 0755);
    if (fd_out < 0)
    {
        ft_printf("Error: Cannot create woody file\n");
        return -1;
    }
    
    // Écrire tout le fichier original jusqu'à la table des sections
    sections_table_offset = pe->dos_header->e_lfanew + sizeof(uint32_t) + 
                            sizeof(t_image_file_header);
    if (pe->arch_type == ELFCLASS32)
        sections_table_offset += sizeof(t_image_optional_header32);
    else
        sections_table_offset += sizeof(t_image_optional_header64);
    
    // Écrire jusqu'à la fin de la dernière section
    if (write(fd_out, pe->base_addr, pe->file_size) != pe->file_size)
    {
        close(fd_out);
        return -1;
    }
    
    // Ajouter du padding pour aligner sur file_alignment
    padding_size = new_section->PointerToRawData - pe->file_size;
    if (padding_size > 0)
    {
        padding = ft_calloc(1, padding_size);
        if (padding)
        {
            write(fd_out, padding, padding_size);
            free(padding);
        }
    }
    
    // Écrire le stub
    if (write(fd_out, stub, new_section->SizeOfRawData) != (ssize_t)new_section->SizeOfRawData)
    {
        close(fd_out);
        return -1;
    }
    
    // Maintenant modifier les headers dans le fichier
    lseek(fd_out, 0, SEEK_SET);
    
    // Réécrire le fichier avec les headers modifiés
    if (write(fd_out, pe->base_addr, pe->file_size) != pe->file_size)
    {
        close(fd_out);
        return -1;
    }
    
    // Ajouter la nouvelle section à la table
    lseek(fd_out, sections_table_offset + 
          (pe->number_of_sections * sizeof(t_image_section_header)), SEEK_SET);
    write(fd_out, new_section, sizeof(t_image_section_header));
    
    // Réécrire le stub avec padding
    lseek(fd_out, new_section->PointerToRawData, SEEK_SET);
    write(fd_out, stub, new_section->SizeOfRawData);
    
    close(fd_out);
    
    ft_printf("key_value: ");
    for (int i = 0; i < KEY_SIZE; i++)
        ft_printf("%02X", (unsigned char)pe->encryption_key[i]);
    ft_printf("\n");
    
    return 0;
}

/*
 * Injecte le payload de déchiffrement dans le PE
 */
void pe_inject(t_pe_file *pe, t_injection_payload *payload)
{
    t_image_section_header new_section;
    unsigned char *stub;
    size_t stub_size;
    
    (void)payload; // Pas utilisé pour PE
    
    if (!pe)
        return;
    
    // Charger le stub assembleur
    stub = load_stub(pe, &stub_size);
    if (!stub)
    {
        ft_printf("Error: Cannot load decryption stub\n");
        return;
    }
    
    // Créer la nouvelle section
    create_woody_section(pe, stub_size, &new_section);
    
    // Patcher le stub avec les bonnes valeurs
    patch_stub(stub, pe, new_section.VirtualAddress);
    
    // Modifier les headers PE en mémoire
    if (pe->arch_type == ELFCLASS32)
    {
        pe->nt_headers32->FileHeader.NumberOfSections++;
        pe->nt_headers32->OptionalHeader.AddressOfEntryPoint = new_section.VirtualAddress;
        pe->nt_headers32->OptionalHeader.SizeOfImage = pe_align(
            new_section.VirtualAddress + new_section.VirtualSize,
            pe->nt_headers32->OptionalHeader.SectionAlignment
        );
    }
    else
    {
        pe->nt_headers64->FileHeader.NumberOfSections++;
        pe->nt_headers64->OptionalHeader.AddressOfEntryPoint = new_section.VirtualAddress;
        pe->nt_headers64->OptionalHeader.SizeOfImage = pe_align(
            new_section.VirtualAddress + new_section.VirtualSize,
            pe->nt_headers64->OptionalHeader.SectionAlignment
        );
    }
    
    // Écrire le nouveau fichier
    if (write_woody_pe(pe, &new_section, stub, stub_size) < 0)
    {
        ft_printf("Error: Failed to write woody PE file\n");
    }
    
    free(stub);
}
