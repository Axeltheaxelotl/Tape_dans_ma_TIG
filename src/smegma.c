#include "woody.h"

/* ************************************************************************** */
/*                          ENDIANNESS CONVERSION                             */
/* ************************************************************************** */

uint16_t get_uint16(uint16_t byte, t_endian endian)
{
    if (endian == LENDIAN)
        return (byte);
    return ((byte << 8) | (byte >> 8));
}

uint32_t get_uint32(uint32_t byte, t_endian endian)
{
    if (endian == LENDIAN)
        return (byte);
    byte = ((byte << 8) & 0xFF00FF00) | ((byte >> 8) & 0xFF00FF);
    return (byte << 16) | (byte >> 16);
}

int32_t get_int32(int32_t byte, t_endian endian)
{
    if (endian == LENDIAN)
        return (byte);
    byte = ((byte << 8) & 0xFF00FF00) | ((byte >> 8) & 0xFF00FF);
    return (byte << 16) | ((byte >> 16) & 0xFFFF);
}

uint64_t get_uint64(uint64_t byte, t_endian endian)
{
    if (endian == LENDIAN)
        return (byte);
    byte = ((byte << 8) & 0xFF00FF00FF00FF00ULL)
        | ((byte >> 8) & 0x00FF00FF00FF00FFULL);
    byte = ((byte << 16) & 0xFFFF0000FFFF0000ULL)
        | ((byte >> 16) & 0x0000FFFF0000FFFFULL);
    return (byte << 32) | (byte >> 32);
}

/* ************************************************************************** */
/*                          SEGMENT VERIFICATION                              */
/* ************************************************************************** */

//verifie si le segment et un segment text (la ou qu il y a le code)
int text(Elf64_Phdr *ph)
{
    if (!ph)
        return 0;
    //il doit etre de type PT_LOAD et PF_X et PF_R
    if ((ph->p_type == PT_LOAD)  && (ph->p_flags & PF_X) && (ph->p_flags & PF_R))
        return 1;
    return 0;
}

//verifie si le segment est data (BSS)
int data(Elf64_Phdr *ph)
{
    if (!ph)
        return 0;
    //PT_LOAD + la dif entre filesz/memsz
    if ((ph->p_type == PT_LOAD) && (ph->p_filesz != ph->p_memsz))
        return 1;
    return 0;
}

//version 32 bites text
int text_32(Elf32_Phdr *ph)
{
    if (!ph)
    {
        return 0;
    }
    if ((ph->p_type == PT_LOAD) && (ph->p_flags & PF_X) && (ph->p_flags & PF_R))
    {
        return 1;
    }
    return 0;
}

int data_32(Elf32_Phdr *ph)
{
    if (!ph)
    {
        return 0;
    }
    if ((ph->p_type == PT_LOAD) && (ph->p_filesz != ph->p_memsz))
    {
        return 1;
    }
    return 0;
}


//esque offset + (nb * taille d une entree) rentre dans un fichier charger en memoire
static int ph_in_buf(void *debut, void *fin, uint64_t offset, uint16_t nb, size_t entree_size)
{
    unsigned char *start = (unsigned char *)debut;
    unsigned char *end = (unsigned char *)fin;
    uint64_t required;

    //taille total buffer
    size_t map_size = (size_t)(end - start);
    
    //offset doit entre dans le fichier !
    if (offset > (uint64_t)map_size)
        return 0;

    //la table entiere (nb * entree_size) tient dans le mapping a partir de l offset
    required = (uint64_t)nb * (uint64_t)entree_size;
    if (required > (uint64_t)(map_size - (size_t)offset))
        return 0;
    return 1;
}




////////////////////////////////////////////////////////////////////////
//version 64 bites

//trouve le permier segment 64 bites qui passse le test
Elf64_Phdr *seg_get64(t_elf_file *file, int (*filetest)(Elf64_Phdr *))
{
    Elf64_Phdr *result = NULL;
    Elf64_Ehdr *eh;
    Elf64_Phdr *tab = NULL;
    uint16_t nb = 0;
    uint64_t phoffset = 0;
    uint16_t i;

    if (!file || !file->base_addr || !file->end_addr || !filetest)
        return NULL;

    //recup le elf header
    eh = (Elf64_Ehdr *)file->base_addr;
    nb = get_uint16(eh->e_phnum, file->endian_type);
    phoffset = get_uint64(eh->e_phoff, file->endian_type);

    //verifie que la table des phdrs est entierment dans le mapping
    if (!ph_in_buf(file->base_addr, file->end_addr, phoffset, nb, sizeof(Elf64_Phdr)))
        return NULL;

    //prepare le pointeur vers table
    tab = (Elf64_Phdr *)((unsigned char *)file->base_addr + phoffset);

    //parcourire la table et appliquer le test
    i = 0;
    while (i < nb)
    {
        Elf64_Phdr *segment_courant = &tab[i];
        //verifie que si l entree courante tient dans le mapping
        if ((unsigned char *)segment_courant + sizeof(Elf64_Phdr) > (unsigned char *)file->end_addr)
        {
            result = NULL;
            break;
        }
        if (filetest(segment_courant))
        {
            result = segment_courant;
            break;
        }
        i++;
    }
    return result;
}

/* ---------- Trouve le dernier PT_LOAD 64 bits (on garde lisibilité) ---------- */
Elf64_Phdr *seg_last64(t_elf_file *file)
{
    Elf64_Phdr *result = NULL;
    Elf64_Ehdr *eh;
    Elf64_Phdr *tab = NULL;
    uint16_t nb = 0;
    uint64_t phoff = 0;
    int i;

    if (!file || !file->base_addr || !file->end_addr)
        return NULL;

    eh = (Elf64_Ehdr *)file->base_addr;
    nb = get_uint16(eh->e_phnum, file->endian_type);
    phoff = get_uint64(eh->e_phoff, file->endian_type);

    if (!ph_in_buf(file->base_addr, file->end_addr, phoff, nb, sizeof(Elf64_Phdr)))
        return NULL;

    tab = (Elf64_Phdr *)((unsigned char *)file->base_addr + phoff);

    /* parcourir en sens inverse : on cherche le dernier PT_LOAD */
    for (i = (int)nb - 1; i >= 0; --i)
    {
        Elf64_Phdr *cur = &tab[i];

        /* vérif que l'entrée est dans le buffer */
        if ((unsigned char *)cur + sizeof(Elf64_Phdr) > (unsigned char *)file->end_addr)
        {
            /* on saute cette entrée mais on continue la recherche */
            continue;
        }

        if (cur->p_type == PT_LOAD)
        {
            result = cur;
            break;
        }
    }

    return result;
}

/* ---------- Versions 32 bits séparées, mêmes étapes ---------- */

Elf32_Phdr *seg_get32(t_elf_file *file, int (*filt)(Elf32_Phdr *))
{
    Elf32_Phdr *result = NULL;
    Elf32_Ehdr *eh;
    Elf32_Phdr *tab = NULL;
    uint16_t nb = 0;
    uint32_t phoff = 0;
    uint16_t i;

    if (!file || !file->base_addr || !file->end_addr || !filt)
        return NULL;

    eh = (Elf32_Ehdr *)file->base_addr;
    nb = get_uint16(eh->e_phnum, file->endian_type);
    phoff = get_uint32(eh->e_phoff, file->endian_type);

    if (!ph_in_buf(file->base_addr, file->end_addr, phoff, nb, sizeof(Elf32_Phdr)))
        return NULL;

    tab = (Elf32_Phdr *)((unsigned char *)file->base_addr + phoff);

    for (i = 0; i < nb; ++i)
    {
        Elf32_Phdr *cur = &tab[i];

        if ((unsigned char *)cur + sizeof(Elf32_Phdr) > (unsigned char *)file->end_addr)
        {
            result = NULL;
            break;
        }

        if (filt(cur))
        {
            result = cur;
            break;
        }
    }

    return result;
}

Elf32_Phdr *seg_last32(t_elf_file *file)
{
    Elf32_Phdr *result = NULL;
    Elf32_Ehdr *eh;
    Elf32_Phdr *tab = NULL;
    uint16_t nb = 0;
    uint32_t phoff = 0;
    int i;

    if (!file || !file->base_addr || !file->end_addr)
        return NULL;

    eh = (Elf32_Ehdr *)file->base_addr;
    nb = get_uint16(eh->e_phnum, file->endian_type);
    phoff = get_uint32(eh->e_phoff, file->endian_type);

    if (!ph_in_buf(file->base_addr, file->end_addr, phoff, nb, sizeof(Elf32_Phdr)))
        return NULL;

    tab = (Elf32_Phdr *)((unsigned char *)file->base_addr + phoff);

    for (i = (int)nb - 1; i >= 0; --i)
    {
        Elf32_Phdr *cur = &tab[i];

        if ((unsigned char *)cur + sizeof(Elf32_Phdr) > (unsigned char *)file->end_addr)
            continue;

        if (cur->p_type == PT_LOAD)
        {
            result = cur;
            break;
        }
    }

    return result;
}

// Wrappers pour correspondre aux noms dans woody.h
Elf64_Phdr *segment(t_elf_file *file, int (*f)(Elf64_Phdr *))
{
    return seg_get64(file, f);
}

Elf64_Phdr *last_load_segment(t_elf_file *file)
{
    return seg_last64(file);
}

Elf32_Phdr *segment_32(t_elf_file *file)
{
    return seg_get32(file, text_32);
}

Elf32_Phdr *get_last_load_segment_32(t_elf_file *file)
{
    return seg_last32(file);
}

int is_text(Elf64_Phdr *phdr)
{
    return text(phdr);
}

int is_data(Elf64_Phdr *phdr)
{
    return data(phdr);
}

int is_text_32(Elf32_Phdr *phdr)
{
    return text_32(phdr);
}

int is_data_32(Elf32_Phdr *phdr)
{
    return data_32(phdr);
}