#include "pe.h"

/*
 * Programme de test simple pour analyser des fichiers PE sur Linux
 * Permet de tester le parser sans exécuter les binaires Windows
 */

void print_usage(const char *progname)
{
    printf("Usage: %s <fichier_pe>\n", progname);
    printf("  Analyse et affiche les informations d'un fichier PE (EXE/DLL Windows)\n");
}

void print_pe_basic_info(t_pe_file *pe)
{
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║          Informations du fichier PE             ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");
    
    printf("📦 Taille fichier:    %ld octets\n", pe->file_size);
    printf("🔧 Architecture:      %s\n", 
           pe->arch_type == ELFCLASS32 ? "PE32 (32-bit)" : "PE32+ (64-bit)");
    printf("🎯 Point d'entrée:    0x%08X (RVA)\n", pe->original_entry_point);
    printf("📂 Nombre sections:   %u\n", pe->number_of_sections);
}

void print_sections_table(t_pe_file *pe)
{
    size_t i;
    t_image_section_header *section;
    
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║                    Sections                      ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");
    
    printf("  %-10s  %-10s  %-10s  %-10s  %s\n", 
           "Nom", "RVA", "VirtSize", "RawSize", "Perms");
    printf("  ───────────────────────────────────────────────────────────\n");
    
    section = pe->section_headers;
    for (i = 0; i < pe->number_of_sections; i++)
    {
        char perms[4];
        perms[0] = (section[i].Characteristics & IMAGE_SCN_MEM_READ) ? 'r' : '-';
        perms[1] = (section[i].Characteristics & IMAGE_SCN_MEM_WRITE) ? 'w' : '-';
        perms[2] = (section[i].Characteristics & IMAGE_SCN_MEM_EXECUTE) ? 'x' : '-';
        perms[3] = '\0';
        
        printf("  %-10s  0x%08X  0x%08X  0x%08X  %s\n",
               section[i].Name,
               section[i].VirtualAddress,
               section[i].VirtualSize,
               section[i].SizeOfRawData,
               perms);
    }
}

void print_text_section_info(t_pe_file *pe)
{
    t_image_section_header *text;
    
    text = pe_find_section(pe, ".text");
    if (!text)
    {
        printf("\n⚠️  Section .text introuvable\n");
        return;
    }
    
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║              Section .text (code)                ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");
    
    printf("  📍 RVA:              0x%08X\n", text->VirtualAddress);
    printf("  📏 Taille virtuelle: 0x%08X (%u octets)\n", 
           text->VirtualSize, text->VirtualSize);
    printf("  💾 Taille sur disk:  0x%08X (%u octets)\n", 
           text->SizeOfRawData, text->SizeOfRawData);
    printf("  📄 Offset fichier:   0x%08X\n", text->PointerToRawData);
    
    // Afficher un mini dump du début
    if (text->SizeOfRawData > 0)
    {
        void *text_data = (char *)pe->base_addr + text->PointerToRawData;
        size_t dump_size = text->SizeOfRawData < 64 ? text->SizeOfRawData : 64;
        
        printf("\n  Premiers octets (hexa):\n");
        printf("  ");
        for (size_t i = 0; i < dump_size && i < 32; i++)
        {
            printf("%02X ", ((unsigned char *)text_data)[i]);
            if (i % 16 == 15)
                printf("\n  ");
        }
        if (dump_size > 32)
            printf("...");
        printf("\n");
    }
}

void run_simple_tests(t_pe_file *pe)
{
    printf("\n╔══════════════════════════════════════════════════╗\n");
    printf("║                  Tests simples                   ║\n");
    printf("╚══════════════════════════════════════════════════╝\n\n");
    
    // Test 1: Vérifier la signature DOS
    if (pe->dos_header && pe->dos_header->e_magic == IMAGE_DOS_SIGNATURE)
        printf("  ✅ Signature DOS valide (MZ)\n");
    else
        printf("  ❌ Signature DOS invalide\n");
    
    // Test 2: Vérifier la section .text
    if (pe_find_section(pe, ".text"))
        printf("  ✅ Section .text trouvée\n");
    else
        printf("  ❌ Section .text manquante\n");
    
    // Test 3: Vérifier le point d'entrée
    if (pe->original_entry_point != 0)
        printf("  ✅ Point d'entrée valide (0x%X)\n", pe->original_entry_point);
    else
        printf("  ⚠️  Point d'entrée à 0 (inhabituel)\n");
    
    // Test 4: Convertir RVA -> offset
    uint32_t entry_offset = pe_rva_to_offset(pe, pe->original_entry_point);
    if (entry_offset != 0)
        printf("  ✅ Conversion RVA->offset OK (0x%X -> 0x%X)\n", 
               pe->original_entry_point, entry_offset);
    else
        printf("  ⚠️  Impossible de convertir le point d'entrée\n");
    
    // Test 5: Compter sections exécutables
    int exec_count = 0;
    for (size_t i = 0; i < pe->number_of_sections; i++)
    {
        if (pe->section_headers[i].Characteristics & IMAGE_SCN_MEM_EXECUTE)
            exec_count++;
    }
    printf("  ℹ️  %d section(s) exécutable(s) trouvée(s)\n", exec_count);
}

/*
 * Parse simplifié qui ne vérifie pas .text (pour les tests)
 */
int pe_simple_parse(const char *filename, t_pe_file *pe)
{
    struct stat st;
    t_image_dos_header *dos;
    t_image_file_header *file_header;
    uint16_t *magic;
    
    // Init structure
    ft_memset(pe, 0, sizeof(t_pe_file));
    
    // Ouvrir le fichier
    pe->file_fd = open(filename, O_RDONLY);
    if (pe->file_fd < 0)
    {
        perror("open");
        return (EXIT_FAILURE);
    }
    
    // Taille du fichier
    if (fstat(pe->file_fd, &st) < 0)
    {
        perror("fstat");
        close(pe->file_fd);
        return (EXIT_FAILURE);
    }
    pe->file_size = st.st_size;
    
    // Mapper le fichier
    pe->base_addr = mmap(NULL, pe->file_size, PROT_READ, MAP_PRIVATE, pe->file_fd, 0);
    if (pe->base_addr == MAP_FAILED)
    {
        perror("mmap");
        close(pe->file_fd);
        return (EXIT_FAILURE);
    }
    pe->end_addr = (char *)pe->base_addr + pe->file_size;
    
    // Vérifier signature DOS
    if (!pe_is_valid(pe->base_addr, pe->file_size))
    {
        printf("❌ Pas un fichier PE valide\n");
        munmap(pe->base_addr, pe->file_size);
        close(pe->file_fd);
        return (EXIT_FAILURE);
    }
    
    dos = (t_image_dos_header *)pe->base_addr;
    pe->dos_header = dos;
    
    // Déterminer l'architecture
    file_header = (t_image_file_header *)((char *)pe->base_addr + dos->e_lfanew + 4);
    if (file_header->Machine == IMAGE_FILE_MACHINE_I386)
        pe->arch_type = ELFCLASS32;
    else if (file_header->Machine == IMAGE_FILE_MACHINE_AMD64)
        pe->arch_type = ELFCLASS64;
    else
    {
        printf("❌ Architecture inconnue: 0x%X\n", file_header->Machine);
        munmap(pe->base_addr, pe->file_size);
        close(pe->file_fd);
        return (EXIT_FAILURE);
    }
    
    // Parser selon l'architecture
    if (pe->arch_type == ELFCLASS32)
    {
        pe->nt_headers32 = (t_image_nt_headers32 *)((char *)pe->base_addr + dos->e_lfanew);
        file_header = &pe->nt_headers32->FileHeader;
        magic = &pe->nt_headers32->OptionalHeader.Magic;
        
        if (*magic != IMAGE_NT_OPTIONAL_HDR32_MAGIC)
        {
            printf("❌ Magic PE32 invalide\n");
            munmap(pe->base_addr, pe->file_size);
            close(pe->file_fd);
            return (EXIT_FAILURE);
        }
        
        pe->original_entry_point = pe->nt_headers32->OptionalHeader.AddressOfEntryPoint;
        pe->section_headers = (t_image_section_header *)((char *)&pe->nt_headers32->OptionalHeader + 
                                                         file_header->SizeOfOptionalHeader);
    }
    else // 64-bit
    {
        pe->nt_headers64 = (t_image_nt_headers64 *)((char *)pe->base_addr + dos->e_lfanew);
        file_header = &pe->nt_headers64->FileHeader;
        magic = &pe->nt_headers64->OptionalHeader.Magic;
        
        if (*magic != IMAGE_NT_OPTIONAL_HDR64_MAGIC)
        {
            printf("❌ Magic PE32+ invalide\n");
            munmap(pe->base_addr, pe->file_size);
            close(pe->file_fd);
            return (EXIT_FAILURE);
        }
        
        pe->original_entry_point = pe->nt_headers64->OptionalHeader.AddressOfEntryPoint;
        pe->section_headers = (t_image_section_header *)((char *)&pe->nt_headers64->OptionalHeader + 
                                                         file_header->SizeOfOptionalHeader);
    }
    
    pe->number_of_sections = file_header->NumberOfSections;
    
    // Chercher .text (optionnel pour les tests)
    t_image_section_header *text = pe_find_section(pe, ".text");
    if (text)
    {
        pe->text_rva = text->VirtualAddress;
        pe->text_size = text->SizeOfRawData;
        pe->text_section = (char *)pe->base_addr + text->PointerToRawData;
    }
    
    return (EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    t_pe_file pe;
    int ret;
    
    if (argc != 2)
    {
        print_usage(argv[0]);
        return (EXIT_FAILURE);
    }
    
    printf("\n🔍 Analyse du fichier: %s\n", argv[1]);
    
    // Parser le fichier PE (version simple pour tests)
    ret = pe_simple_parse(argv[1], &pe);
    if (ret != EXIT_SUCCESS)
    {
        printf("❌ Erreur: impossible de parser le fichier\n");
        return (EXIT_FAILURE);
    }
    
    // Afficher les infos
    print_pe_basic_info(&pe);
    print_sections_table(&pe);
    print_text_section_info(&pe);
    run_simple_tests(&pe);
    
    printf("\n✨ Analyse terminée avec succès!\n\n");
    
    // Cleanup
    pe_cleanup(&pe);
    
    return (EXIT_SUCCESS);
}
