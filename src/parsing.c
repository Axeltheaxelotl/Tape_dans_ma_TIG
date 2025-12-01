#include "woody.h"

/*
** check_file : Ouvre, mappe et valide un fichier ELF
** --------------------------------------------------
** Cette fonction prend en entrée un nom de fichier et un pointeur vers une structure t_elf_file.
** Elle réalise toutes les étapes nécessaires pour charger un fichier ELF en mémoire et vérifier
** qu'il s'agit bien d'un binaire ELF valide, exécutable ou dynamique, pour les architectures x86/x86_64.
**
** Détail des étapes :
** 1. Ouvre le fichier en lecture seule. Si l'ouverture échoue, retourne une erreur.
** 2. Récupère la taille du fichier avec lseek. Si échec, retourne une erreur.
** 3. Mappe le fichier en mémoire (lecture/écriture privée). Si échec, retourne une erreur.
** 4. Calcule l'adresse de fin du mapping.
** 5. Récupère un pointeur sur le début du mapping (ident) et le cast en en-tête ELF (header).
** 6. Récupère l'architecture (32/64 bits) et l'endianness depuis l'en-tête ELF.
** 7. Prépare une erreur générique ENOEXEC si le format n'est pas reconnu.
** 8. Vérifie la magie ELF (0x7F 'E' 'L' 'F') pour s'assurer que le fichier est bien un ELF.
** 9. Vérifie la classe (32/64 bits), l'endianness et la version ELF.
** 10. Vérifie que la machine cible est bien x86 ou x86_64.
** 11. Vérifie la version de l'en-tête ELF.
** 12. Vérifie qu'il y a au moins un programme header (section de segments).
** 13. Vérifie que le type de fichier est exécutable (ET_EXEC) ou dynamique (ET_DYN).
** 14. Si toutes les vérifications passent, retourne EXIT_SUCCESS, sinon EXIT_FAILURE.
**
** Cette fonction permet de sécuriser le traitement des fichiers ELF en s'assurant que seuls
** les binaires valides et compatibles sont acceptés pour la suite du programme.
*/
int le_paaarsing(const char *filename, t_elf_file *file)
{
    unsigned char   *ident;
    Elf64_Ehdr      *header;
    char            magic[4] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

    errno = 0;
    // 1. Ouvre le fichier en lecture seule
    file->file_fd = open(filename, O_RDONLY);
    if (file->file_fd == -1)
        return (EXIT_FAILURE);

    // 2. Récupère la taille du fichier
    file->file_size = lseek(file->file_fd, 0, SEEK_END);
    if (file->file_size == (off_t)(-1))
        return (EXIT_FAILURE);

    // 3. Mappe le fichier en mémoire (lecture + écriture privée)
    file->base_addr = mmap(NULL, file->file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, file->file_fd, 0);
    if (file->base_addr == MAP_FAILED)
        return (EXIT_FAILURE);

    // 4. Calcule la fin du fichier en mémoire
    file->end_addr = (void *)((char *)file->base_addr + file->file_size);

    // 5. Pointeur sur le début du fichier mappé
    ident = (unsigned char *)file->base_addr;
    header = (Elf64_Ehdr *)file->base_addr;

    // 6. Récupère l'architecture (32/64 bits) et l'endianness
    file->arch_type = ident[EI_CLASS];
    file->endian_type = ident[EI_DATA];

    // 7. Prépare une erreur générique si le format n'est pas reconnu
    errno = ENOEXEC;

    // 8. Vérifie la magie ELF (0x7F 'E' 'L' 'F')
    if (ft_memcmp(magic, ident, 4))
        return (EXIT_FAILURE);

    // 9. Vérifie la classe (32/64 bits), l'endianness, la version ELF
    if (ident[EI_CLASS] == ELFCLASSNONE || ident[EI_DATA] == ELFDATANONE || ident[EI_VERSION] == EV_NONE)
        return (EXIT_FAILURE);

    // 10. Vérifie le type de machine (x86 ou x86_64)
    if (!(get_uint16(header->e_machine, file->endian_type) == EM_386 ||
          get_uint16(header->e_machine, file->endian_type) == EM_X86_64))
        return (EXIT_FAILURE);

    // 11. Vérifie la version de l'en-tête ELF
    if (get_uint32(header->e_version, file->endian_type) == EV_NONE)
        return (EXIT_FAILURE);

    // 12. Vérifie qu'il y a au moins un programme header
    if (get_uint16(header->e_phnum, file->endian_type) == 0)
        return (EXIT_FAILURE);

    // 13. Vérifie que le type de fichier est exécutable ou dynamique
    if (header->e_type != ET_EXEC && header->e_type != ET_DYN)
        return (EXIT_FAILURE);

    // 14. Si toutes les vérifications passent, le fichier ELF est valide
    return (EXIT_SUCCESS);
}