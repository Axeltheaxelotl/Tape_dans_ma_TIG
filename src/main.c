#include "woody.h"

/*
** Affiche l'usage du programme et retourne EXIT_FAILURE
** Utilisé quand les arguments sont incorrects
*/
static int usage(void)
{
    ft_putstr_fd("Usage: ./woody_woodpacker [-k <key>] <binary>\n", 2);
    ft_putstr_fd("  -k <key>  : Specify encryption key (64 hex chars for 32 bytes)\n", 2);
    ft_putstr_fd("              Example: -k 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n", 2);
    ft_putstr_fd("  <binary>  : ELF binary to encrypt (32 or 64 bits)\n", 2);
    return (EXIT_FAILURE);
}

/*
** Parse les arguments de la ligne de commande
** Retourne l'index du fichier binaire, ou -1 en cas d'erreur
** Si -k est fourni, parse la clé et la stocke dans file->taille_key
*/
static int parse_arguments(int argc, char **argv, t_elf_file *file)
{
    int binary_index;

    // Cas 1: ./woody_woodpacker <binary>
    // Génération aléatoire de la clé
    if (argc == 2)
    {
        file->is_key_provided = 0;  // Clé aléatoire
        binary_index = 1;
        return (binary_index);
    }

    // Cas 2: ./woody_woodpacker -k <key> <binary>
    // Utilisation d'une clé fournie par l'utilisateur
    if (argc == 4)
    {
        // Vérifier que le flag -k est présent
        if (ft_strcmp(argv[1], "-k") != 0)
            return (-1);

        // Parser la clé hexadécimale (64 caractères -> 32 bytes)
        if (parse_key_from_string(argv[2], file->taille_key) == EXIT_FAILURE)
        {
            ft_putstr_fd("Error: Invalid key format. Key must be 64 hexadecimal characters.\n", 2);
            return (-1);
        }

        file->is_key_provided = 1;  // Clé fournie par l'utilisateur
        binary_index = 3;
        return (binary_index);
    }

    // Mauvais nombre d'arguments
    return (-1);
}

int main(int argc, char **argv)
{
    t_elf_file fichier;
    t_injection_payload payload;
    int size;
    int binary_index;

    // Mise à zéro de la structure fichier pour éviter les valeurs indéfinies
    ft_bzero(&fichier, sizeof(t_elf_file));
    errno = 0;

    // Parse les arguments et récupère l'index du fichier binaire
    binary_index = parse_arguments(argc, argv, &fichier);
    if (binary_index == -1)
        return (usage());

    // Vérification de la validité du fichier ELF
    if (le_paaarsing(argv[binary_index], &fichier) == EXIT_FAILURE)
        error_w(&fichier, NULL, NULL, !errno ? ERROR_ARGS : ERROR_ERRNO);

    // Chiffrement de la section .text du fichier ELF
    encryptitation_code(&fichier);

    // Construction du payload à injecter, selon l'architecture du fichier
    if (fichier.arch_type == ELFCLASS64)
    {
        size = CODE_SIZE;
        // Prépare la structure payload avec les offsets spécifiques 64 bits
        payload = (t_injection_payload){
            malloc(size), size, 0x1e, 0x23, 0x2a, 0x78, 0x5c};
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
        payload = (t_injection_payload){
            malloc(size), size, 0x1d, 0x22, 0x2e, 0x70, 0x54};
        // Copie le code assembleur de déchiffrement 32 bits dans le buffer
        ft_memcpy(payload.payload_code, DECRYPTION_CODE_32, size);
    }

    // Injection du code de déchiffrement et de la clé dans le fichier ELF
    injectitation(&fichier, &payload);

    // Libération de la mémoire allouée pour le code du payload
    free(payload.payload_code);
    return 0;
}
