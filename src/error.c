
#include "woody.h" // Inclusion du header principal du projet



static void print_error_message(int code)
{
    const char *msg = NULL; // Pointeur vers le message d'erreur
    if (code == 1) // Cas 1 : erreur sur les headers du programme
    {
        msg = "program headers extends past the end of the file.";
    }
    else if (code == 2) // Cas 2 : erreur sur le segment texte
    {
        msg = "text segment extends past the end of the file.";
    }
    else if (code == 3) // Cas 3 : mauvais nombre d'arguments
    {
        msg = "Wrong number of arguments.\n\tusage: ./woody_woodpacker filename";
    }
    else // Cas par défaut : pas de message
    {
        msg = "";
    }
    if (msg && *msg) // Si un message est défini et non vide
        dprintf(2, "woody_woodpacker: error: %s\n", msg); // Affiche sur la sortie d'erreur
}


// Fonction principale de gestion d'erreur
// Libère les ressources et affiche un message avant de quitter le programme
void	error_w(t_elf_file *file, t_injection_payload *payload, t_elf_segments *woody, int code)
{
    // 1. Affichage du message d'erreur
    if (code != ERROR_ERRNO) // Si ce n'est pas une erreur système
        print_error_message(code); // Affiche un message personnalisé selon le code
    else
        perror("woody_woodpacker: error"); // Affiche le message d'erreur système lié à errno

    // 2. Libération du mapping mémoire du fichier ELF
    if (file->base_addr) // Si le pointeur mémoire du fichier est valide
        munmap(file->base_addr, file->file_size); // On libère la mémoire mappée
    // 3. Fermeture du descripteur de fichier
    if (file->file_fd) // Si le descripteur de fichier est ouvert
        close(file->file_fd); // On le ferme
    // 4. Libération du buffer du payload (code injecté)
    if (payload) // Si le payload existe
        free(payload->payload_code); // On libère la mémoire du code injecté
    // 5. Libération de la mémoire principale de woody
    if (woody) // Si la structure woody existe
        free(woody->base_ptr); // On libère la mémoire associée
    // 6. Fin du programme avec code d'échec
    exit(EXIT_FAILURE); // Quitte le programme avec un code d'erreur
}