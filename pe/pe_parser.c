#include "pe.h"

/*
 * Vérifie si un buffer contient un fichier PE valide
 */
int pe_is_valid(const void *base, size_t size)
{
    t_image_dos_header *dos;
    uint32_t *pe_signature;
    
    if (size < sizeof(t_image_dos_header))
        return (0);
    
    dos = (t_image_dos_header *)base;
    
    // Vérifier la signature MZ
    if (dos->e_magic != IMAGE_DOS_SIGNATURE)
        return (0);
    
    // Vérifier que e_lfanew pointe dans le fichier
    if (dos->e_lfanew < 0 || (size_t)dos->e_lfanew + 4 > size)
        return (0);
    
    // Vérifier la signature PE
    pe_signature = (uint32_t *)((char *)base + dos->e_lfanew);
    if (*pe_signature != IMAGE_NT_SIGNATURE)
        return (0);
    
    return (1);
}

/*
 * Cherche une section par nom
 */
t_image_section_header *pe_find_section(t_pe_file *pe, const char *name)
{
    t_image_section_header *section;
    size_t i;
    
    if (!pe || !pe->section_headers || !name)
        return (NULL);
    
    section = pe->section_headers;
    i = 0;
    while (i < pe->number_of_sections)
    {
        if (ft_strncmp((char *)section[i].Name, name, IMAGE_SIZEOF_SHORT_NAME) == 0)
            return (&section[i]);
        i++;
    }
    return (NULL);
}

/*
 * Aligne une valeur selon l'alignement spécifié
 */
uint32_t pe_align(uint32_t value, uint32_t alignment)
{
    if (alignment == 0)
        return (value);
    return ((value + alignment - 1) / alignment) * alignment;
}

/*
 * Convertit une RVA (Relative Virtual Address) en offset fichier
 */
uint32_t pe_rva_to_offset(t_pe_file *pe, uint32_t rva)
{
    t_image_section_header *section;
    size_t i;
    
    if (!pe || !pe->section_headers)
        return (0);
    
    section = pe->section_headers;
    i = 0;
    while (i < pe->number_of_sections)
    {
        if (rva >= section[i].VirtualAddress &&
            rva < section[i].VirtualAddress + section[i].VirtualSize)
        {
            return (section[i].PointerToRawData + (rva - section[i].VirtualAddress));
        }
        i++;
    }
    return (0);
}

/*
 * Parse les structures PE et remplit la structure t_pe_file
 */
static int parse_pe_structures(t_pe_file *pe)
{
    t_image_file_header *file_header;
    uint16_t *magic;
    
    // Récupérer le DOS header
    pe->dos_header = (t_image_dos_header *)pe->base_addr;
    
    // Pointer vers les NT headers
    if (pe->arch_type == ELFCLASS32)
    {
        pe->nt_headers32 = (t_image_nt_headers32 *)((char *)pe->base_addr + pe->dos_header->e_lfanew);
        file_header = &pe->nt_headers32->FileHeader;
        magic = &pe->nt_headers32->OptionalHeader.Magic;
        
        // Vérifier le magic
        if (*magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        {
            ft_putstr_fd("Error: Invalid PE32 magic number\n", 2);
            return (EXIT_FAILURE);
        }
        
        // Récupérer le point d'entrée et autres infos
        pe->original_entry_point = pe->nt_headers32->OptionalHeader.AddressOfEntryPoint;
        
        // Pointer vers la table des sections
        pe->section_headers = (t_image_section_header *)((char *)&pe->nt_headers32->OptionalHeader + 
                                                         file_header->SizeOfOptionalHeader);
    }
    else // 64-bit
    {
        pe->nt_headers64 = (t_image_nt_headers64 *)((char *)pe->base_addr + pe->dos_header->e_lfanew);
        file_header = &pe->nt_headers64->FileHeader;
        magic = &pe->nt_headers64->OptionalHeader.Magic;
        
        // Vérifier le magic
        if (*magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        {
            ft_putstr_fd("Error: Invalid PE32+ magic number\n", 2);
            return (EXIT_FAILURE);
        }
        
        // Récupérer le point d'entrée
        pe->original_entry_point = pe->nt_headers64->OptionalHeader.AddressOfEntryPoint;
        
        // Pointer vers la table des sections
        pe->section_headers = (t_image_section_header *)((char *)&pe->nt_headers64->OptionalHeader + 
                                                         file_header->SizeOfOptionalHeader);
    }
    
    pe->number_of_sections = file_header->NumberOfSections;
    return (EXIT_SUCCESS);
}

/*
 * Trouve et configure la section .text
 */
static int find_text_section(t_pe_file *pe)
{
    t_image_section_header *text_section;
    
    // Chercher la section .text
    text_section = pe_find_section(pe, ".text");
    if (!text_section)
    {
        ft_putstr_fd("Error: .text section not found\n", 2);
        return (EXIT_FAILURE);
    }
    
    // Vérifier que la section est exécutable
    if (!(text_section->Characteristics & IMAGE_SCN_MEM_EXECUTE))
    {
        ft_putstr_fd("Error: .text section is not executable\n", 2);
        return (EXIT_FAILURE);
    }
    
    // Stocker les infos de la section .text
    pe->text_rva = text_section->VirtualAddress;
    pe->text_size = text_section->SizeOfRawData;
    pe->text_section = (char *)pe->base_addr + text_section->PointerToRawData;
    
    // Vérifier que la section est dans le fichier
    if ((char *)pe->text_section + pe->text_size > (char *)pe->end_addr)
    {
        ft_putstr_fd("Error: .text section extends beyond file\n", 2);
        return (EXIT_FAILURE);
    }
    
    return (EXIT_SUCCESS);
}

/*
 * Parse un fichier PE et remplit la structure t_pe_file
 */
int pe_parse(const char *filename, t_pe_file *pe)
{
    struct stat st;
    t_image_file_header *file_header;
    
    // Ouvrir le fichier
    pe->file_fd = open(filename, O_RDONLY);
    if (pe->file_fd < 0)
    {
        perror("open");
        return (EXIT_FAILURE);
    }
    
    // Obtenir la taille du fichier
    if (fstat(pe->file_fd, &st) < 0)
    {
        perror("fstat");
        close(pe->file_fd);
        return (EXIT_FAILURE);
    }
    pe->file_size = st.st_size;
    
    // Mapper le fichier en mémoire
    pe->base_addr = mmap(NULL, pe->file_size, PROT_READ, MAP_PRIVATE, pe->file_fd, 0);
    if (pe->base_addr == MAP_FAILED)
    {
        perror("mmap");
        close(pe->file_fd);
        return (EXIT_FAILURE);
    }
    pe->end_addr = (char *)pe->base_addr + pe->file_size;
    
    // Vérifier que c'est un PE valide
    if (!pe_is_valid(pe->base_addr, pe->file_size))
    {
        ft_putstr_fd("Error: Not a valid PE file\n", 2);
        munmap(pe->base_addr, pe->file_size);
        close(pe->file_fd);
        return (EXIT_FAILURE);
    }
    
    // Déterminer l'architecture
    pe->dos_header = (t_image_dos_header *)pe->base_addr;
    file_header = (t_image_file_header *)((char *)pe->base_addr + pe->dos_header->e_lfanew + 4);
    
    if (file_header->Machine == IMAGE_FILE_MACHINE_I386)
        pe->arch_type = ELFCLASS32;
    else if (file_header->Machine == IMAGE_FILE_MACHINE_AMD64)
        pe->arch_type = ELFCLASS64;
    else
    {
        ft_putstr_fd("Error: Unsupported architecture. Only x86 and x64 are supported.\n", 2);
        munmap(pe->base_addr, pe->file_size);
        close(pe->file_fd);
        return (EXIT_FAILURE);
    }
    
    // Parser les structures PE
    if (parse_pe_structures(pe) == EXIT_FAILURE)
    {
        munmap(pe->base_addr, pe->file_size);
        close(pe->file_fd);
        return (EXIT_FAILURE);
    }
    
    // Trouver la section .text
    if (find_text_section(pe) == EXIT_FAILURE)
    {
        munmap(pe->base_addr, pe->file_size);
        close(pe->file_fd);
        return (EXIT_FAILURE);
    }
    
    return (EXIT_SUCCESS);
}

/*
 * Nettoie les ressources d'un fichier PE
 */
void pe_cleanup(t_pe_file *pe)
{
    if (!pe)
        return;
    
    if (pe->base_addr && pe->base_addr != MAP_FAILED)
        munmap(pe->base_addr, pe->file_size);
    
    if (pe->file_fd >= 0)
        close(pe->file_fd);
}
