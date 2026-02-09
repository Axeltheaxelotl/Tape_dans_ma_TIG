#include "woody.h"

static void encryptitation_code_32(t_elf_file *file)
{
    void *text;
    void *cle;
    uint32_t taille_text;
    uint32_t taille_cle;
    Elf32_Phdr *phdr;

    verbose_printf("Location .text segment (32bits)...\n");
    //trouver le segment .text dans la table des program headers
    file->section_sex = segment_32(file);
    if(file->section_sex == NULL)
    {
        verbose_printf("Error: .text segment not found\n");
        error_w(file, NULL, NULL, ERROR_PH_TRUNC);
        return ;
    }
    verbose_printf(".text segement found\n");


    //ajouter le flag d ecriture (PF_W) au segment
    // On prend le premier Program Header d’un fichier ELF en mémoire.
    // On modifie ses permissions pour ajouter la permission d’écriture.
    // Cela ne modifie pas le fichier sur disque tant qu’on ne l’écrit pas, mais en mémoire ça change comment le loader pourrait mapper ce segment.
    phdr = (Elf32_Phdr *)file->section_sex;
    phdr->p_flags |= PF_W;
    verbose_printf("Added write permission to .text segement\n");
    //calculer le pointeur vers le code chiffrer
    uint32_t offset = get_uint32(phdr->p_offset, file->endian_type);
    text = file->base_addr + offset;

    //recup la taille du code a chiffrer
    taille_text = get_uint32(phdr->p_filesz, file->endian_type);
    verbose_printf(".text section offset : 0x%x\n", offset);
    verbose_printf(".text section size: %u bytes\n", taille_text);
    //generer une putain de cle aleatoire
    cle_aleatoire(file);

    //preparer les parametres pour encryptitation
    cle = &file->taille_key;
    taille_cle = KEY_SIZE;

    //verifier que la zone texte est dans la limites du fichier
    if (text < file->base_addr || text > file->end_addr || (text + taille_text) > file->end_addr)
    {
        verbose_printf("Error: .text section out ou bounds\n");
        error_w(file, NULL, NULL, ERROR_TEXT_TRUNC);
        return ;
    }
    verbose_printf(".text section encrypted successfully\n");

    //chiffrer la zone contenant le code
    encryptitation(cle, taille_cle, text, taille_text); //assembler de met couille on verra bieng
}


//pour le 64 bites mtn
static void encryptitation_code_64(t_elf_file *file)
{
    void *text;
    void *cle;
    uint64_t taille_text;
    uint32_t taille_cle;
    Elf64_Phdr *phdr;
    verbose_printf("Location .text segment (64bit)...\n");
    //trouver le segment .text dans la table des program headers
    file->section_sex = segment(file, is_text);
    if(file->section_sex == NULL)
    {
        verbose_printf(".text sgement not found\n");
        error_w(file, NULL, NULL, ERROR_PH_TRUNC);
        return ;
    }
    verbose_printf(".text sgement found\n");

    //ajouter le flag d ecriture (PF_W) au segment
    phdr = (Elf64_Phdr *)file->section_sex;
    phdr->p_flags |= PF_W;
    verbose_printf(".text segment found\n");
    //calculer le pointeur vers le code chiffrer
    uint64_t offset = get_uint64(phdr->p_offset, file->endian_type);
    text = file->base_addr + offset;

    //recup la taille du code a chiffrer
    taille_text = get_uint64(phdr->p_filesz, file->endian_type);

    verbose_printf(".text section offset: 0x%lx\n", offset);
    verbose_printf(".text section size: %lu bytes\n", taille_text);
    //generer une putain de cle aleatoire
    cle_aleatoire(file);

    //preparer les parametres pour encryptitation
    cle = &file->taille_key;
    taille_cle = KEY_SIZE;

    //verifier que la zone texte est dans la limites du fichier
    if (text < file->base_addr || text > file->end_addr || (text + taille_text) > file->end_addr)
    {
        verbose_printf("Error: .text section out of bounds\n");
        error_w(file, NULL, NULL, ERROR_TEXT_TRUNC);
        return ;
    }
    verbose_printf("Encrypting: .text section (%lu bytes)...\n", taille_text);
    //chiffrer la zone contenant le code
    encryptitation(cle, taille_cle, text, taille_text);
    verbose_printf(".text section encrypted sucessfully\n");
}






///////////////////////////////////////////////////////////////////////////////////////////

/*injectitation*/

//ecrit le nouveau binaire de facon atomique
static int save_fichie(char *buf, size_t size)
{
    int fd;
    ssize_t ecrit;
    int ret;

    //cree et ouvrir un fichier temporaire en ecriture perm 0755
    errno = 0;
    fd = open("tamere.tmp", O_WRONLY | O_CREAT | O_TRUNC, 0755);
    if (fd == -1)
        return EXIT_FAILURE;

    //ecrit tout le fils de pute de buffer dans le tamere
    ecrit = 0;
    while((ssize_t)ecrit < (ssize_t)size)
    {
        ssize_t w = write(fd, buf + ecrit, size - ecrit);
        if(w == -1)
        {
            close(fd);
            //enleve le tamere "si possible"
            syscall(SYS_unlink, "tamere.tmp");
            return EXIT_FAILURE;
        }
        ecrit += w;
    }


    //fermer le fichier tmp
    if (close(fd) == -1)
    {
        syscall(SYS_unlink, "tamere.tmp");
        return EXIT_FAILURE;
    }

    //renomation de tamere.tmp
    ret = syscall(SYS_rename, "tamere.tmp", "woody");
    if (ret != 0)
    {
        //bas si pas marcher toi essayer de exploser tmp
        syscall(SYS_unlink, "tamere.tmp");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}





//remplit les putes de valeur manquantes dans le playload
static void format_playload_32_bites(t_elf_file *file, t_injection_payload *playload, Elf32_Addr entry_addr)
{
    Elf32_Ehdr *elf_header;
    uint32_t entre_originale;
    int32_t jmp_back_offset;
    int32_t text_offset;
    uint32_t text_filesize;
    uint32_t taille_cle;


    elf_header = (Elf32_Ehdr *)file->base_addr;
    
    //lire l adresse d entree originale (e_entry) du binaire ELF
    entre_originale = get_uint32(elf_header->e_entry, file->endian_type);
    
    /*
        calcul du saut relatif pour revenir a la putain d entree original:
        offset_jump pointe vers l'immédiat du jmp (après l'opcode 0xe9)
        La prochaine instruction est à offset_jump + 4 (taille de l'immédiat)
        rel = old_entry - (adresse_du_playload + offset_du_jmp + 4)    
    */
    jmp_back_offset = (int32_t)(entre_originale) - ((int32_t)entry_addr + playload->offset_jump + 4);

    /*
        calcul de l offset relatif vers le debut de la section .text :
        En 32-bit, après call/pop, edx = entry_addr + 0x2b (position après pop à 0x2b)
        On ajoute ensuite un offset avec add edx, imm32 (offset de l'immediate = 0x2e)
        On veut: (entry_addr + 0x2b) + offset = text_vaddr
        Donc: offset = text_vaddr - (entry_addr + 0x2b)
    */
    text_offset = (int32_t)(get_uint32(((Elf32_Phdr *)file->section_sex)->p_vaddr, file->endian_type) - (entry_addr + 0x2b));


    //taille reelle du segment .text
    text_filesize = get_uint32(((Elf32_Phdr *)file->section_sex)->p_filesz, file->endian_type);

    //taille de la cle  embarquee dans le playload
    taille_cle = KEY_SIZE;

    // ecriture des valeure  calculee dans les zones du payload 
    ft_memcpy(playload->payload_code + playload->offset_text_size, &text_filesize, sizeof(uint32_t));
    ft_memcpy(playload->payload_code + playload->offset_key_size, &taille_cle, sizeof(uint32_t));
    ft_memcpy(playload->payload_code + playload->offset_key, &file->taille_key, KEY_SIZE);
    ft_memcpy(playload->payload_code + playload->offset_text, &text_offset, sizeof(int32_t));
    ft_memcpy(playload->payload_code + playload->offset_jump, &jmp_back_offset, sizeof(int32_t));
}



//pour le 64 bites mtn
static void format_payload_64_bites(t_elf_file *file, t_injection_payload *payload, Elf64_Addr entry_addr)
{
    Elf64_Ehdr *elf_header;
    uint64_t entre_originale;
    int32_t jmp_back_offset;
    int32_t text_offset;
    uint64_t text_filesize;
    uint32_t taille_cle;


    elf_header = (Elf64_Ehdr *)file->base_addr;

    // lire l'adresse d'entrée originale (e_entry) du binaire ELF
    entre_originale = get_uint64(elf_header->e_entry, file->endian_type);

    /*
        calcul du saut relatif pour revenir à l'entrée originale:
        offset_jump pointe vers l'immédiat du jmp (après l'opcode 0xe9)
        La prochaine instruction est à offset_jump + 4 (taille de l'immédiat)
        rel = old_entry - (adresse_du_payload + offset_du_jmp + 4)
    */
    jmp_back_offset = (int32_t)(entre_originale) - ((int32_t)entry_addr + payload->offset_jump + 4);

    /*
        calcul de l'offset relatif vers le début de la section .text :
        En 64-bit, on utilise lea [rel routine], qui est RIP-relative
        Après lecture de l'instruction lea (7 bytes), rip = entry_addr + offset_text + 4
        On veut: (entry_addr + offset_text + 4) + rel_offset = text_vaddr
        Donc: rel_offset = text_vaddr - (entry_addr + offset_text + 4)
    */
    text_offset = (int32_t)(get_uint64(((Elf64_Phdr *)file->section_sex)->p_vaddr, file->endian_type) - ((int64_t)entry_addr + payload->offset_text + 4));

    // taille réelle du segment .text
    text_filesize = get_uint64(((Elf64_Phdr *)file->section_sex)->p_filesz, file->endian_type);

    // taille de la clé embarquée dans le payload
    taille_cle = KEY_SIZE;

    // écriture des valeurs calculées dans les zones du payload
    ft_memcpy(payload->payload_code + payload->offset_text_size, &text_filesize, sizeof(uint32_t));
    ft_memcpy(payload->payload_code + payload->offset_key_size, &taille_cle, sizeof(uint32_t));
    ft_memcpy(payload->payload_code + payload->offset_key, &file->taille_key, KEY_SIZE);
    ft_memcpy(payload->payload_code + payload->offset_text, &text_offset, sizeof(int32_t));
    ft_memcpy(payload->payload_code + payload->offset_jump, &jmp_back_offset, sizeof(int32_t));
}


static void creation_du_woody_de_met_couille_32_bits(t_elf_file *file, t_elf_segments *woody, size_t *mettre_a_index)
{
    size_t file_index;
    uint32_t data_segment_offset;
    uint32_t data_segment_filesz;
    uint32_t data_segment_memsz;
    uint32_t last_segment_offset;
    uint32_t last_segment_filesz;

    //calcul l index ou on injectera le payload a la fin du segment data
    // - on calcul ou, dans le fichier, il faudra ajoutee des donnees
    // - woody->data_segment_32->p_offset: position de debut d un segment dans le fichier
    // - woody->data_data_segment_32->p_filesz: taille de ce segment dans le fichier
    // - addition des deux pour obtenir la position juste apres ce segment
    // - garadation de la position dans file_index
    data_segment_offset = get_uint32(woody->data_segment_32->p_offset, file->endian_type);
    data_segment_filesz = get_uint32(woody->data_segment_32->p_filesz, file->endian_type);
    *mettre_a_index = (size_t)data_segment_offset + (size_t)data_segment_filesz;
    file_index = *mettre_a_index;

    //copie tout le debut du fichier original jusqu a la position calculee juste avt
    //woody->base_ptr le nouveau fichie en memoire
    //file->base_addr le le fichier original
    ft_memcpy(woody->base_ptr, file->base_addr, *mettre_a_index);


    //si la taille du segment en memoire p_memsz est plus grand que sur le disque p_filesz
    //si oui, faut mettre des zeros "remplire" avec des octets a zero pour combler la dif
    //met a jour la position d injectation
    //ajustation du pointeur data_32 pour qu il pointe vers la bonne position dans le new file
    //met a jour la taille du disque pour qu elle corresponde a la taille memoire
    data_segment_memsz = get_uint32(woody->data_segment_32->p_memsz, file->endian_type);
    if (woody->data_segment_32 && data_segment_filesz != data_segment_memsz)
    {
        size_t pad = (size_t)data_segment_memsz - (size_t)data_segment_filesz;
        ft_bzero(woody->base_ptr + *mettre_a_index, pad);
        *mettre_a_index += pad;
        //corrige ptr
        woody->data_segment_32 = (Elf32_Phdr *)((unsigned char *)woody->base_ptr + ((unsigned char *)woody->data_segment_32 - (unsigned char *)file->base_addr));
        woody->data_segment_32->p_filesz = woody->data_segment_32->p_memsz;
    }

    //si last_segment_32 et data_segment_32 sont differents, copier la zon entre file_index et la fin du last segment
    //si le dernier segment courant data_segment_32 il faut copier la partie du fichier comprise entre la fin du segment courant et la fin du dernier segment
    //on calcul la taille copier copy_taille
    //avance jusqu a la position d injection
    //et copie de cette partie du fichier original dans le new buffer
    last_segment_offset = get_uint32(woody->last_segment_32->p_offset, file->endian_type);
    last_segment_filesz = get_uint32(woody->last_segment_32->p_filesz, file->endian_type);
    if (woody->last_segment_32 != woody->data_segment_32)
    {
        size_t copy_size = (size_t)last_segment_offset + (size_t)last_segment_filesz - file_index;
        ft_memcpy((unsigned char *)woody->base_ptr + *mettre_a_index, (unsigned char *)file->base_addr + file_index, copy_size);
        *mettre_a_index += copy_size;
    }

    //ajustation du ptr last_segment_32 pour qu il pointe dans le nouveau buffer, et non plus dans l ancien fichier
    woody->last_segment_32 = (Elf32_Phdr *)((unsigned char *)woody->base_ptr + ((unsigned char *)woody->last_segment_32 - (unsigned char *)file->base_addr));
}

static void creation_du_woody_de_met_couille_64_bits(t_elf_file *file, t_elf_segments *woody, size_t *mettre_a_index)
{
    size_t file_index;
    uint64_t data_segment_offset;
    uint64_t data_segment_filesz;
    uint64_t data_segment_memsz;
    uint64_t last_segment_offset;
    uint64_t last_segment_filesz;

    // calcul de l'index d'injection à la fin du segment data
    data_segment_offset = get_uint64(woody->data_segment->p_offset, file->endian_type);
    data_segment_filesz = get_uint64(woody->data_segment->p_filesz, file->endian_type);
    *mettre_a_index = (size_t)data_segment_offset + (size_t)data_segment_filesz;
    file_index = *mettre_a_index;

    // copie du début du fichier original jusqu'à la position d'injection
    ft_memcpy(woody->base_ptr, file->base_addr, *mettre_a_index);

    // padding si p_memsz > p_filesz
    data_segment_memsz = get_uint64(woody->data_segment->p_memsz, file->endian_type);
    if (woody->data_segment && data_segment_filesz != data_segment_memsz)
    {
        size_t pad = (size_t)data_segment_memsz - (size_t)data_segment_filesz;
        ft_bzero((unsigned char *)woody->base_ptr + *mettre_a_index, pad);
        *mettre_a_index += pad;
        // corrige le pointeur
        woody->data_segment = (Elf64_Phdr *)((unsigned char *)woody->base_ptr + ((unsigned char *)woody->data_segment - (unsigned char *)file->base_addr));
        woody->data_segment->p_filesz = woody->data_segment->p_memsz;
    }

    // copie la zone entre la fin du segment data et la fin du dernier segment si différents
    last_segment_offset = get_uint64(woody->last_segment->p_offset, file->endian_type);
    last_segment_filesz = get_uint64(woody->last_segment->p_filesz, file->endian_type);
    if (woody->last_segment != woody->data_segment)
    {
        size_t copy_size = (size_t)last_segment_offset + (size_t)last_segment_filesz - file_index;
        ft_memcpy((unsigned char *)woody->base_ptr + *mettre_a_index, (unsigned char *)file->base_addr + file_index, copy_size);
        *mettre_a_index += copy_size;
    }

    // ajuste le pointeur last_segment pour le nouveau buffer
    woody->last_segment = (Elf64_Phdr *)((unsigned char *)woody->base_ptr + ((unsigned char *)woody->last_segment - (unsigned char *)file->base_addr));
}





/*
CREE UN NOUVEAU ELF:
    1. selectionnant un segment ou injecter le payload
    2. etendant ce segment si necessaire
    3. recalculant toutes les tailles
    4. copiant le binaire d origine dans le new buffer
    5. injectant ton payload a la fin d un segment PT_LOAD
    6. modifiant le point d entree pour pointer vers le code
    7. sauvegardant le nouvel elf patche
*/

static void j_te_met_32(t_elf_file *file, t_injection_payload *payload)
{
    t_elf_segments woody;
    size_t j_te_la_met;
    size_t besoin_taille;
    Elf32_Phdr *last32;

    //recup le dernier segment PT_LOAD (la ou je la met en fin de segment)
    last32 = get_last_load_segment_32(file);
    if(last32 == NULL)
    {
        error_w(file, payload, NULL, ERROR_PH_TRUNC);
        return ;
    }
    woody.last_segment_32 = last32;

    //trouve le segment "data" ou utiliser le dernier si absent
    //on cherche le segment qui contient les donnees (data)
    //si pas trouver on utilise le dernier segment trouver avt
    woody.data_segment_32 = seg_get32(file, is_data_32);
    if (!woody.data_segment_32)
    {
        woody.data_segment_32 = woody.last_segment_32;
    }


    //calcul la taille qui faut pour le new binaire
    //pour calculer la taille il faut
    /*
        - la taille tu fichier original jusqu a la fin du dernier segment de met couilles
        - la taille du code a injecter (payload)
        - plus "si besoin" l espace a ajouter pour que la taille du en memoire et sur le disque du segment soir coherente
    */
    besoin_taille = (size_t)get_uint32(woody.last_segment_32->p_offset, file->endian_type) + (size_t)get_uint32(woody.last_segment_32->p_filesz, file->endian_type) + payload->payload_size + ((size_t)get_uint32(woody.last_segment_32->p_memsz, file->endian_type) - (size_t)get_uint32(woody.data_segment_32->p_filesz, file->endian_type));
    woody.total_size = besoin_taille;

    //alloue le new buffer
    woody.base_ptr = malloc(woody.total_size);
    if(!woody.base_ptr)
    {
        error_w(file, payload, &woody, ERROR_ERRNO);
        return ;
    }

    //preparer le payload (remplir cles, offsets relatif etc ...)
    format_playload_32_bites(file, payload, get_uint32(woody.last_segment_32->p_vaddr, file->endian_type) + get_uint32(woody.last_segment_32->p_memsz, file->endian_type));

    //constructation du new buffer et obtenir j te la met l index de l injection
    //copie la partie du fichier origianal dans le new bufferen ajustatant les segments si besoin, on obtient la position exacte ou il faut placer le code a injecter
    creation_du_woody_de_met_couille_32_bits(file, &woody, &j_te_la_met);

    //copier le code du payload a l endroit de l injectation
    ft_memcpy(woody.base_ptr + j_te_la_met, payload->payload_code, payload->payload_size);

    //modifie le point d entree du programme
    uint32_t new_entry_32 = get_uint32(woody.last_segment_32->p_vaddr, file->endian_type) + get_uint32(woody.last_segment_32->p_memsz, file->endian_type);
    ((Elf32_Ehdr *)woody.base_ptr)->e_entry = get_uint32(new_entry_32, file->endian_type);
    

    //mettre a jour la taille du dernier segment
    //augement la taille du segment pour inclure le code a injecter, sur le disque et en memoire
    uint32_t new_filesz_32 = get_uint32(woody.last_segment_32->p_filesz, file->endian_type) + payload->payload_size;
    uint32_t new_memsz_32 = get_uint32(woody.last_segment_32->p_memsz, file->endian_type) + payload->payload_size;
    woody.last_segment_32->p_filesz = get_uint32(new_filesz_32, file->endian_type);
    woody.last_segment_32->p_memsz = get_uint32(new_memsz_32, file->endian_type);

    //mettre le segment executable
    //p_flags = les permissions du segment PF_R = lecture PF_W = ecriture PF_X = execution
    woody.last_segment_32->p_flags |= PF_X;

    //ecrit le nouveau fichier sur le disque si ca echoue, on affiche une erreur et on libere la memoire
    if (save_fichie(woody.base_ptr, woody.total_size) == EXIT_FAILURE)
    {
        error_w(file, payload, &woody, ERROR_ERRNO);
        free(woody.base_ptr);
        return ;
    }
}

static void j_te_met_64(t_elf_file *file, t_injection_payload *payload)
{
    t_elf_segments woody;
    size_t j_te_la_met;
    size_t besoin_taille;
    Elf64_Phdr *last64;

    //recup le dernier segment PT_LOAD (la ou je la met en fin de segment)
    last64 = last_load_segment(file);
    if(last64 == NULL)
    {
        error_w(file, payload, NULL, ERROR_PH_TRUNC);
        return ;
    }
    woody.last_segment = last64;

    //trouve le segment "data" ou utiliser le dernier si absent
    woody.data_segment = segment(file, is_data);
    if (!woody.data_segment)
    {
        woody.data_segment = woody.last_segment;
    }

    //calcul la taille qui faut pour le new binaire
    besoin_taille = (size_t)get_uint64(woody.last_segment->p_offset, file->endian_type) + (size_t)get_uint64(woody.last_segment->p_filesz, file->endian_type) + payload->payload_size + ((size_t)get_uint64(woody.data_segment->p_memsz, file->endian_type) - (size_t)get_uint64(woody.data_segment->p_filesz, file->endian_type));
    woody.total_size = besoin_taille;

    //alloue le new buffer
    woody.base_ptr = malloc(woody.total_size);
    if(!woody.base_ptr)
    {
        error_w(file, payload, &woody, ERROR_ERRNO);
        return ;
    }

    //preparer le payload (remplir cles, offsets relatif etc ...)
    format_payload_64_bites(file, payload, get_uint64(woody.last_segment->p_vaddr, file->endian_type) + get_uint64(woody.last_segment->p_memsz, file->endian_type));

    //constructation du new buffer et obtenir j te la met l index de l injectation
    creation_du_woody_de_met_couille_64_bits(file, &woody, &j_te_la_met);

    //copier le code du payload a l endroit de l injectation
    ft_memcpy(woody.base_ptr + j_te_la_met, payload->payload_code, payload->payload_size);

    //modifie le point d entree du programme
    uint64_t new_entry = get_uint64(woody.last_segment->p_vaddr, file->endian_type) + get_uint64(woody.last_segment->p_memsz, file->endian_type);
    ((Elf64_Ehdr *)woody.base_ptr)->e_entry = get_uint64(new_entry, file->endian_type);
    
    //mettre a jour la taille du dernier segment
    uint64_t new_filesz = get_uint64(woody.last_segment->p_filesz, file->endian_type) + payload->payload_size;
    uint64_t new_memsz = get_uint64(woody.last_segment->p_memsz, file->endian_type) + payload->payload_size;
    woody.last_segment->p_filesz = get_uint64(new_filesz, file->endian_type);
    woody.last_segment->p_memsz = get_uint64(new_memsz, file->endian_type);

    //mettre le segment executable
    woody.last_segment->p_flags |= PF_X;

    //ecrit le nouveau fichier sur le disque
    if (save_fichie(woody.base_ptr, woody.total_size) == EXIT_FAILURE)
    {
        error_w(file, payload, &woody, ERROR_ERRNO);
        free(woody.base_ptr);
        return ;
    }
}

void injectitation(t_elf_file *file, t_injection_payload *payload)
{
    if (file->arch_type == ELFCLASS32)
        j_te_met_32(file, payload);
    else if (file->arch_type == ELFCLASS64)
        j_te_met_64(file, payload);
}

void encryptitation_code(t_elf_file *file)
{
    if (file->arch_type == ELFCLASS32)
        encryptitation_code_32(file);
    else if (file->arch_type == ELFCLASS64)
        encryptitation_code_64(file);
}