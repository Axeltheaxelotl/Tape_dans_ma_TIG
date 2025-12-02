#include "woody.h"


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
    if ((ph->p_type == PT_LOAD) && (ph->p_flags & PF_X) && (ph->flags & PF_R))
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
    if ((ph->type == PT_LOAD) && (ph->p_filesz != ph->p_memsz))
    {
        return 1;
    }
    return 0;
}


//esque offset + (nb * taille d une entree) rentre dans un fichier charger en memoire
static int ph_in_buf(void *debut, void *fin, uint64_t offset, uin16_t nb, size_t entree_size)
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
    uin16_t nb = 0;
    uint64_t phoffset = 0
    uin16_t i;

    if (!file || !file->base_addr || !f->end_addr || !filetest)
        return NULL;

    //recup le elf header
    eh = (Elf64_Ehdr *)file->base_ptr;
    nb = eh->e_phnum;
    phoffset = eh->e_phoff;

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
        
        i++;
    }
}