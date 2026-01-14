#include "woody.h"
#include "pe.h"

// Enumération pour identifier le type de fichier
typedef enum e_file_type
{
    FILE_TYPE_UNKNOWN = 0,
    FILE_TYPE_ELF,
    FILE_TYPE_PE
} t_file_type;

/*
** Affiche l'usage du programme et retourne EXIT_FAILURE
** Utilisé quand les arguments sont incorrects
*/
static int usage(void)
{
    ft_putstr_fd("Usage: ./woody_woodpacker [-k <key>] <binary>\n", 2);
    ft_putstr_fd("  -k <key>  : Specify encryption key (64 hex chars for 32 bytes)\n", 2);
    ft_putstr_fd("              Example: -k 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\n", 2);
    ft_putstr_fd("  <binary>  : ELF or PE binary to encrypt (32 or 64 bits)\n", 2);
    return (EXIT_FAILURE);
}

/*
** Détecte le type de fichier (ELF ou PE)
** En lisant les premiers bytes (magic numbers)
*/
static t_file_type detect_file_type(const char *filename)
{
    int fd;
    unsigned char buffer[4];
    ssize_t bytes_read;
    t_file_type type;

    type = FILE_TYPE_UNKNOWN;
    fd = open(filename, O_RDONLY);
    if (fd < 0)
        return (type);

    bytes_read = read(fd, buffer, 4);
    if (bytes_read == 4)
    {
        // Check ELF magic: 0x7F 'E' 'L' 'F'
        if (buffer[0] == 0x7F && buffer[1] == 'E' && 
            buffer[2] == 'L' && buffer[3] == 'F')
            type = FILE_TYPE_ELF;
        // Check PE magic: 'M' 'Z' (DOS header)
        else if (buffer[0] == 'M' && buffer[1] == 'Z')
            type = FILE_TYPE_PE;
    }

    close(fd);
    return (type);
}

/*
** Parse les arguments de la ligne de commande
** Retourne l'index du fichier binaire, ou -1 en cas d'erreur
** Si -k est fourni, parse la clé et la stocke dans key_buffer
*/
static int parse_arguments(int argc, char **argv, char *key_buffer, int *is_key_provided)
{
    int binary_index;

    // Cas 1: ./woody_woodpacker <binary>
    // Génération aléatoire de la clé
    if (argc == 2)
    {
        *is_key_provided = 0;  // Clé aléatoire
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
        if (parse_key_from_string(argv[2], key_buffer) == EXIT_FAILURE)
        {
            ft_putstr_fd("Error: Invalid key format. Key must be 64 hexadecimal characters.\n", 2);
            return (-1);
        }

        *is_key_provided = 1;  // Clé fournie par l'utilisateur
        binary_index = 3;
        return (binary_index);
    }

    // Mauvais nombre d'arguments
    return (-1);
}

/*
** Traite un fichier ELF
*/
static int process_elf_file(const char *filename, char *key_buffer, int is_key_provided)
{
    t_elf_file fichier;
    t_injection_payload payload;
    int size;

    // Mise à zéro de la structure fichier pour éviter les valeurs indéfinies
    ft_bzero(&fichier, sizeof(t_elf_file));
    
    // Copier la clé et le flag
    ft_memcpy(fichier.taille_key, key_buffer, KEY_SIZE);
    fichier.is_key_provided = is_key_provided;

    // Vérification de la validité du fichier ELF
    if (le_paaarsing(filename, &fichier) == EXIT_FAILURE)
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
        payload = (t_injection_payload){
            malloc(size), size, 0x1d, 0x22, 0x2e, 0x70, 0x54};
        // Copie le code assembleur de déchiffrement 32 bits dans le buffer
        ft_memcpy(payload.payload_code, DECRYPTION_CODE_32, size);
    }

    // Injection du code de déchiffrement et de la clé dans le fichier ELF
    injectitation(&fichier, &payload);

    // Libération de la mémoire allouée pour le code du payload
    free(payload.payload_code);
    return (EXIT_SUCCESS);
}

/*
** Traite un fichier PE
*/
static int process_pe_file(const char *filename, char *key_buffer, int is_key_provided)
{
    t_pe_file pe;
    t_injection_payload payload;
    int size;

    // Initialisation de la structure PE
    ft_bzero(&pe, sizeof(t_pe_file));
    
    // Copier la clé et le flag
    ft_memcpy(pe.encryption_key, key_buffer, KEY_SIZE);
    pe.is_key_provided = is_key_provided;

    // Parser le fichier PE
    if (pe_parse(filename, &pe) == EXIT_FAILURE)
    {
        ft_putstr_fd("Error: Failed to parse PE file\n", 2);
        return (EXIT_FAILURE);
    }

    ft_printf("PE file detected: %s (%d-bit)\n", filename, 
              pe.arch_type == ELFCLASS64 ? 64 : 32);

    // Chiffrer la section .text
    pe_encrypt(&pe);

    // Construire le payload selon l'architecture
    if (pe.arch_type == ELFCLASS64)
    {
        size = CODE_SIZE; // TODO: Utiliser PE_CODE_SIZE_64 quand le code sera prêt
        payload = (t_injection_payload){
            malloc(size), size, 0x1e, 0x23, 0x2a, 0x78, 0x5c};
        ft_memcpy(payload.payload_code, DECRYPTION_CODE, size); // TODO: Utiliser PE_DECRYPTION_CODE_64
    }
    else
    {
        size = CODE_SIZE_32; // TODO: Utiliser PE_CODE_SIZE_32 quand le code sera prêt
        payload = (t_injection_payload){
            malloc(size), size, 0x1d, 0x22, 0x2e, 0x70, 0x54};
        ft_memcpy(payload.payload_code, DECRYPTION_CODE_32, size); // TODO: Utiliser PE_DECRYPTION_CODE_32
    }

    // Injecter le code dans le PE
    pe_inject(&pe, &payload);

    // Nettoyage
    free(payload.payload_code);
    pe_cleanup(&pe);

    return (EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    char key_buffer[KEY_SIZE];
    int is_key_provided;
    int binary_index;
    t_file_type file_type;

    // Initialisation
    ft_bzero(key_buffer, KEY_SIZE);
    errno = 0;

    // Parse les arguments et récupère l'index du fichier binaire
    binary_index = parse_arguments(argc, argv, key_buffer, &is_key_provided);
    if (binary_index == -1)
        return (usage());

    // Détecter le type de fichier
    file_type = detect_file_type(argv[binary_index]);
    
    if (file_type == FILE_TYPE_ELF)
    {
        ft_printf("Processing ELF file: %s\n", argv[binary_index]);
        return (process_elf_file(argv[binary_index], key_buffer, is_key_provided));
    }
    else if (file_type == FILE_TYPE_PE)
    {
        ft_printf("Processing PE file: %s\n", argv[binary_index]);
        return (process_pe_file(argv[binary_index], key_buffer, is_key_provided));
    }
    else
    {
        ft_putstr_fd("Error: Unknown file format. Only ELF and PE files are supported.\n", 2);
        return (EXIT_FAILURE);
    }
}
