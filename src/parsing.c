#include "woody.h"

static int validate_elf_32(t_elf_file *file)
{
    Elf32_Ehdr *header32;

    header32 = (Elf32_Ehdr *)file->base_addr;
    verbose_printf("Validating 32-but ELF headers...\n");
    if (!(get_uint16(header32->e_machine, file->endian_type) == EM_386))
    {
        verbose_printf("Error: machine type is not EM_386\n");
        return (EXIT_FAILURE);
    }
    if (get_uint32(header32->e_version, file->endian_type) == EV_NONE)
    {
        verbose_printf("Error: invalid ELF version\n");    
        return (EXIT_FAILURE);
    }
    if (get_uint16(header32->e_phnum, file->endian_type) == 0)
    {
        verbose_printf("Error: No program headers found\n");    
        return (EXIT_FAILURE);
    }
    if (get_uint16(header32->e_type, file->endian_type) != ET_EXEC && 
        get_uint16(header32->e_type, file->endian_type) != ET_DYN)
    {
        verbose_printf("Error: file type is not executable or shared object\n");
        return (EXIT_FAILURE);
    }
    verbose_printf("32-bit ELF validation successful\n");
    verbose_printf("Programme headers: %d\n", get_uint16(header32->e_phnum, file->endian_type));
    verbose_printf("Entry point: 0x%x\n", get_uint32(header32->e_entry, file->endian_type));
    return (EXIT_SUCCESS);
}

static int validate_elf_64(t_elf_file *file)
{
    Elf64_Ehdr *header64;

    header64 = (Elf64_Ehdr *)file->base_addr;
    verbose_printf("Validating 64-bit ELF headers...\n");
    if (!(get_uint16(header64->e_machine, file->endian_type) == EM_X86_64))
    {
        verbose_printf("Error: Machine type is not EM_X86_64\n");
        return (EXIT_FAILURE);
    }
    if (get_uint32(header64->e_version, file->endian_type) == EV_NONE)
    {
        verbose_printf("Error: invalid ELF version\n");    
        return (EXIT_FAILURE);
    }
    if (get_uint16(header64->e_phnum, file->endian_type) == 0)
    {
        verbose_printf("Error: no program headers founds\n");    
        return (EXIT_FAILURE);
    }
    if (get_uint16(header64->e_type, file->endian_type) != ET_EXEC && 
        get_uint16(header64->e_type, file->endian_type) != ET_DYN)
    {
        verbose_printf("Error: file type is not executable or shared object\n");
        return (EXIT_FAILURE);
    }
    verbose_printf("64-bit ELF validation sucessful\n");
    verbose_printf("Program headers: %d\n", get_uint16(header64->e_phnum, file->endian_type));
    verbose_printf("Entry point: 0x%lx\n", get_uint64(header64->e_entry, file->endian_type));
    return (EXIT_SUCCESS);
}

int le_paaarsing(const char *filename, t_elf_file *file)
{
    unsigned char   *ident;
    char            magic[4] = {ELFMAG0, ELFMAG1, ELFMAG2, ELFMAG3};
    verbose_printf("Opening file: %s\n", filename);
    errno = 0;
    file->file_fd = open(filename, O_RDONLY);
    if (file->file_fd == -1)
    {
        verbose_printf("Error: Cannot open file: %s\n", strerror(errno));
        return (EXIT_FAILURE);
    }
    verbose_printf("File opened successfully (fd=%d)\n", file->file_fd);
    verbose_printf("Getting file size...\n");
    file->file_size = lseek(file->file_fd, 0, SEEK_END);
    if (file->file_size == (off_t)(-1))
    {
        verbose_printf("Error: cannot open file size: %s\n", strerror(errno));
        return (EXIT_FAILURE);
    }
    verbose_printf("File size: %ld bytes\n", file->file_size);
    verbose_printf("Mapping file into memory...\n");
    file->base_addr = mmap(NULL, file->file_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, file->file_fd, 0);
    if (file->base_addr == MAP_FAILED)
    {
        verbose_printf("Error: mmap failed: %s\n", strerror(errno));    
        return (EXIT_FAILURE);
    }
    verbose_printf("File mapped at address: %p\n", file->base_addr);
    file->end_addr = (void *)((char *)file->base_addr + file->file_size);

    ident = (unsigned char *)file->base_addr;

    file->arch_type = ident[EI_CLASS];
    file->endian_type = ident[EI_DATA];

    verbose_printf("Checking ELF magic number...\n");

    errno = ENOEXEC;

    if (ft_memcmp(magic, ident, 4))
    {
        verbose_printf("Error: Not a valid ELF file (bad magic number)\n");
        return (EXIT_FAILURE);
    }
    verbose_printf("ELF magic number valid\n");
    verbose_printf("Architecture: %s\n", ident[EI_CLASS] == ELFCLASS32 ? "32-bit" : "64-bit");
    verbose_printf("Endianness: %s\n",ident[EI_DATA] == ELFDATA2LSB ? "Little Endian" : "Big Endian");
    if (ident[EI_CLASS] == ELFCLASSNONE || ident[EI_DATA] == ELFDATANONE || ident[EI_VERSION] == EV_NONE)
    {
        verbose_printf("Error: Invalid ELF class, data, or version\n");
        return (EXIT_FAILURE);
    }
    if (ident[EI_CLASS] == ELFCLASS32)
        return (validate_elf_32(file));
    else if (ident[EI_CLASS] == ELFCLASS64)
        return (validate_elf_64(file));
    verbose_printf("Error: Unknown ELF class\n");
    return (EXIT_FAILURE);
}