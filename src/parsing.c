#include "woody.h"

static int validate_elf_32(t_elf_file *file)
{
    Elf32_Ehdr *header32;

    header32 = (Elf32_Ehdr *)file->base_addr;
    
    if (!(get_uint16(header32->e_machine, file->endian_type) == EM_386))
        return (EXIT_FAILURE);

    if (get_uint32(header32->e_version, file->endian_type) == EV_NONE)
        return (EXIT_FAILURE);

    if (get_uint16(header32->e_phnum, file->endian_type) == 0)
        return (EXIT_FAILURE);

    if (get_uint16(header32->e_type, file->endian_type) != ET_EXEC && 
        get_uint16(header32->e_type, file->endian_type) != ET_DYN)
        return (EXIT_FAILURE);

    return (EXIT_SUCCESS);
}

static int validate_elf_64(t_elf_file *file)
{
    Elf64_Ehdr *header64;

    header64 = (Elf64_Ehdr *)file->base_addr;

    if (!(get_uint16(header64->e_machine, file->endian_type) == EM_X86_64))
        return (EXIT_FAILURE);

    if (get_uint32(header64->e_version, file->endian_type) == EV_NONE)
        return (EXIT_FAILURE);

    if (get_uint16(header64->e_phnum, file->endian_type) == 0)
        return (EXIT_FAILURE);

    if (get_uint16(header64->e_type, file->endian_type) != ET_EXEC && 
        get_uint16(header64->e_type, file->endian_type) != ET_DYN)
        return (EXIT_FAILURE);

    return (EXIT_SUCCESS);
}

int le_paaarsing(const char *filename, t_elf_file *file)
{
    unsigned char   *ident;
    char            magic[4] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};

    errno = 0;
    file->file_fd = open(filename, O_RDONLY);
    if (file->file_fd == -1)
        return (EXIT_FAILURE);

    file->file_size = lseek(file->file_fd, 0, SEEK_END);
    if (file->file_size == (off_t)(-1))
        return (EXIT_FAILURE);

    file->base_addr = mmap(NULL, file->file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, file->file_fd, 0);
    if (file->base_addr == MAP_FAILED)
        return (EXIT_FAILURE);

    file->end_addr = (void *)((char *)file->base_addr + file->file_size);

    ident = (unsigned char *)file->base_addr;

    file->arch_type = ident[EI_CLASS];
    file->endian_type = ident[EI_DATA];

    errno = ENOEXEC;

    if (ft_memcmp(magic, ident, 4))
        return (EXIT_FAILURE);

    if (ident[EI_CLASS] == ELFCLASSNONE || ident[EI_DATA] == ELFDATANONE || ident[EI_VERSION] == EV_NONE)
        return (EXIT_FAILURE);

    if (ident[EI_CLASS] == ELFCLASS32)
        return (validate_elf_32(file));
    else if (ident[EI_CLASS] == ELFCLASS64)
        return (validate_elf_64(file));
    
    return (EXIT_FAILURE);
}