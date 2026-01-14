#include "pe.h"

/*
 * Chiffre la section .text d'un fichier PE
 * Utilise le même algorithme que pour les ELF
 */
void pe_encrypt(t_pe_file *pe)
{
    void *text_to_encrypt;
    size_t size_to_encrypt;
    
    if (!pe || !pe->text_section)
        return;
    
    // Générer la clé si nécessaire
    if (!pe->is_key_provided)
    {
        // Réutiliser la fonction de génération de clé ELF
        // On cast vers t_elf_file car la structure est compatible pour la clé
        cle_aleatoire((t_elf_file *)pe);
    }
    
    // Pointer vers la section .text à chiffrer
    text_to_encrypt = pe->text_section;
    size_to_encrypt = pe->text_size;
    
    // Appliquer le chiffrement (XOR)
    encryptitation(text_to_encrypt, size_to_encrypt, pe->encryption_key, KEY_SIZE);
    
    ft_printf("PE .text section encrypted: %zu bytes\n", size_to_encrypt);
}
