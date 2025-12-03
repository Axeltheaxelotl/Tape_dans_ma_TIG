#include "woody.h"

int main(int argc, char **argv)
{
    t_elf_file fichier;
    t_injection_payload payload;
    int size;

    // Mise à zéro de la structure fichier pour éviter les valeurs indéfinies
    ft_bzero(&fichier, sizeof(t_elf_file));
    errno = 0;

    // Vérification du nombre d'arguments et de la validité du fichier ELF
    if (argc != 2 || le_paaarsing(argv[1], &fichier) == EXIT_FAILURE)
        error_w(&fichier, NULL, NULL, !errno ? ERROR_ARGS : ERROR_ERRNO);

    // Chiffrement de la section .text du fichier ELF
    encryptitation_code(&fichier);

    // Construction du payload à injecter, selon l'architecture du fichier
    if (fichier.arch_type == ELFCLASS64)
    {
        size = CODE_SIZE;
        // Prépare la structure payload avec les offsets spécifiques 64 bits
        payload = (t_injection_payload){malloc(size), size, 0x1e, 0x23, 0x2a, 0x78, 0x5c};
        // Copie le code assembleur de déchiffrement 64 bits dans le buffer
        ft_memcpy(payload.payload_code, DECRYPTION_CODE, size);
    }
    else
    {
        size = CODE_SIZE_32;
        // Prépare la structure payload avec les offsets spécifiques 32 bits
        // offset_text_size: 0x1d (mov ecx, imm32) - immediate à 0x1d
        // offset_key_size: 0x22 (mov esi, imm32) - immediate à 0x22
        // offset_text: 0x2e (add edx, imm32) - immediate à 0x2e (instruction add à 0x2c)
        // offset_key: 0x70 (clé de 32 bytes)
        // offset_jump: 0x54 (jmp imm32) - immediate à 0x54
        payload = (t_injection_payload){malloc(size), size, 0x1d, 0x22, 0x2e, 0x70, 0x54};
        // Copie le code assembleur de déchiffrement 32 bits dans le buffer
        ft_memcpy(payload.payload_code, DECRYPTION_CODE_32, size);
    }

    // Injection du code de déchiffrement et de la clé dans le fichier ELF
    injectitation(&fichier, &payload);

    // Libération de la mémoire allouée pour le code du payload
    free(payload.payload_code);
    return 0;
}