/* ************************************************************************** */
/*                                                                            */
/*   key_management.c                                                         */
/*                                                                            */
/*   Gestion des clés de chiffrement pour woody_woodpacker                   */
/*   - Génération aléatoire via /dev/random                                  */
/*   - Parsing de clé hexadécimale fournie par l'utilisateur                 */
/*   - Validation et conversion hexadécimal -> bytes                         */
/*                                                                            */
/* ************************************************************************** */

#include "woody.h"


static size_t key_strlen(const char *str)
{
    size_t len;

    len = 0;
    while (str[len])
        len++;
    return (len);
}

/*
** Convertit un caractère hexadécimal en sa valeur numérique (0-15)
** 
** Paramètres:
**   c : Caractère à convertir
**
** Retourne:
**   0-9   pour les caractères '0'-'9'
**   10-15 pour les caractères 'A'-'F' ou 'a'-'f'
**   -1    si le caractère n'est pas hexadécimal
**
** Exemples:
**   '0' -> 0
**   '9' -> 9
**   'A' ou 'a' -> 10
**   'F' ou 'f' -> 15
**   'G' -> -1 (invalide)
**
** Note:
**   Cette fonction est insensible à la casse (accepte majuscules et minuscules)
*/
static int hex_char_to_value(char c)
{
    // Chiffres 0-9
    if (c >= '0' && c <= '9')
        return (c - '0');
    
    // Lettres majuscules A-F
    if (c >= 'A' && c <= 'F')
        return (c - 'A' + 10);
    
    // Lettres minuscules a-f
    if (c >= 'a' && c <= 'f')
        return (c - 'a' + 10);
    
    // Caractère invalide
    return (-1);
}

int parse_key_from_string(const char *key_str, char *key_buffer)
{
    int i;
    int high_nibble;
    int low_nibble;

    // Validation 1: Vérifier la longueur de la chaîne
    // KEY_SIZE = 32 bytes, chaque byte = 2 caractères hex
    // Donc on attend exactement 32 * 2 = 64 caractères
    if (key_strlen(key_str) != KEY_SIZE * 2)
        return (EXIT_FAILURE);

    // Conversion: traiter chaque paire de caractères hexadécimaux
    i = 0;
    while (i < KEY_SIZE)
    {
        // Extraire et convertir le nibble de poids fort (4 bits supérieurs)
        // Position dans la chaîne: i * 2
        high_nibble = hex_char_to_value(key_str[i * 2]);
        if (high_nibble == -1)
            return (EXIT_FAILURE);  // Caractère invalide détecté

        // Extraire et convertir le nibble de poids faible (4 bits inférieurs)
        // Position dans la chaîne: i * 2 + 1
        low_nibble = hex_char_to_value(key_str[i * 2 + 1]);
        if (low_nibble == -1)
            return (EXIT_FAILURE);  // Caractère invalide détecté

        // Combiner les deux nibbles pour former un byte complet
        // Opération bit à bit:
        //   - high_nibble << 4 : Décale de 4 bits vers la gauche
        //   - | low_nibble     : OU binaire avec le nibble de poids faible
        // Exemple: high=0x0A, low=0x0B
        //   (0x0A << 4) = 0xA0
        //   0xA0 | 0x0B = 0xAB
        key_buffer[i] = (char)((high_nibble << 4) | low_nibble);
        
        i++;
    }

    return (EXIT_SUCCESS);
}


static void generate_random_key(t_elf_file *file)
{
    int fd;
    ssize_t nread;

    // Étape 1: Ouvrir /dev/random
    // O_RDONLY = ouverture en lecture seule
    fd = open("/dev/random", O_RDONLY);
    if (fd == -1)
    {
        // Échec d'ouverture: errno est automatiquement défini par open()
        error_w(file, NULL, NULL, ERROR_ERRNO);
        return ;
    }

    // Étape 2: Lire KEY_SIZE (32) bytes aléatoires
    // read() retourne le nombre de bytes effectivement lus
    nread = read(fd, file->taille_key, KEY_SIZE);

    // Vérifier les erreurs de lecture
    if (nread < 0)
    {
        // Erreur système (errno défini par read())
        close(fd);
        error_w(file, NULL, NULL, ERROR_ERRNO);
        return ;
    }

    // Vérifier que tous les bytes ont été lus
    if (nread != KEY_SIZE)
    {
        // Lecture incomplète (ne devrait jamais arriver avec /dev/random)
        close(fd);
        error_w(file, NULL, NULL, ERROR_ERRNO);
        return ;
    }

    // Étape 3: Fermer le descripteur de fichier
    if (close(fd) == -1)
    {
        // Échec de fermeture (rare mais possible)
        error_w(file, NULL, NULL, ERROR_ERRNO);
        return ;
    }

    // Succès: La clé aléatoire est maintenant dans file->taille_key
}

/*
** Affiche la clé de chiffrement en format hexadécimal
** 
** Format de sortie:
**   "key_value: 0A1B2C3D4E5F6A7B8C9D0E1F2A3B4C5D6E7F8A9B0C1D2E3F4A5B6C7D8E9F0A1B\n"
**
** Détails d'affichage:
**   - Préfixe: "key_value: "
**   - Chaque byte affiché en hexadécimal majuscule sur 2 chiffres
**   - Padding avec zéros si nécessaire (ex: 0x05 -> "05")
**   - Suffixe: newline '\n'
**
** Paramètres:
**   file : Structure contenant la clé (file->taille_key)
**
** Format printf:
**   %02hhX signifie:
**     %    : Début du format
**     02   : Largeur minimale de 2 caractères, padding avec '0'
**     hh   : Modificateur pour type 'char' (1 byte)
**     X    : Hexadécimal majuscule (A-F au lieu de a-f)
**
** Exemples:
**   0x05 -> "05"
**   0xAB -> "AB"
**   0xFF -> "FF"
*/
static void display_key(t_elf_file *file)
{
    int i;

    // Afficher le préfixe
    ft_putstr_fd("key_value: ", 1);

    // Afficher chaque byte de la clé en hexadécimal
    i = 0;
    while (i < KEY_SIZE)
    {
        // %02hhX: format hexadécimal sur 2 chiffres, type char, majuscules
        printf("%02hhX", file->taille_key[i]);
        i++;
    }
    
    // Afficher le newline final
    printf("\n");
}

/*
** Gère la clé de chiffrement (génération ou utilisation)
** 
** Cette fonction centralise toute la logique de gestion de clé:
**   - Génération aléatoire si aucune clé n'est fournie
**   - Utilisation de la clé existante si fournie par l'utilisateur
**   - Affichage de la clé dans tous les cas
**
** Deux modes de fonctionnement:
**
** MODE 1: Clé aléatoire (is_key_provided == 0)
**   Cas: ./woody_woodpacker <binary>
**   Actions:
**     1. Générer une nouvelle clé via /dev/random
**     2. Stocker dans file->taille_key
**     3. Afficher la clé générée
**   Avantages:
**     - Sécurité maximale (clé unique par exécution)
**     - Entropie cryptographique garantie
**     - Comportement par défaut sécurisé
**
** MODE 2: Clé paramétrisée (is_key_provided == 1)
**   Cas: ./woody_woodpacker -k <key_hex> <binary>
**   Actions:
**     1. La clé est déjà parsée et présente dans file->taille_key
**        (parsée par parse_key_from_string() dans main.c)
**     2. Afficher la clé fournie
**   Avantages:
**     - Déterminisme (mêmes entrées = mêmes sorties)
**     - Reproductibilité pour les tests
**     - Debugging facilité
**
** Paramètres:
**   file : Structure ELF contenant:
**            - is_key_provided : Flag du mode (0=aléatoire, 1=paramétré)
**            - taille_key      : Buffer de 32 bytes pour la clé
**
** Flux d'exécution:
**   ┌─────────────────────────────────┐
**   │ cle_aleatoire(file)             │
**   └─────────────┬───────────────────┘
**                 │
**                 ▼
**   ┌─────────────────────────────────┐
**   │ is_key_provided == 0 ?          │
**   └─────┬───────────────────┬───────┘
**         │ OUI               │ NON
**         ▼                   ▼
**   ┌──────────────┐    ┌────────────────┐
**   │ Générer clé  │    │ Clé déjà       │
**   │ aléatoire    │    │ présente       │
**   │ (random)     │    │ (paramétré)    │
**   └──────┬───────┘    └────────┬───────┘
**          │                     │
**          └──────────┬──────────┘
**                     ▼
**            ┌──────────────────┐
**            │ Afficher la clé  │
**            └──────────────────┘
**
** Note importante:
**   En mode paramétré (is_key_provided == 1), cette fonction
**   NE GÉNÈRE PAS de clé. Elle suppose que la clé a déjà été
**   parsée et validée par parse_key_from_string().
*/
void cle_aleatoire(t_elf_file *file)
{
    // MODE 1: Génération aléatoire
    // Si aucune clé n'a été fournie par l'utilisateur
    if (file->is_key_provided == 0)
    {
        // Générer une nouvelle clé aléatoire via /dev/random
        // Cette fonction peut bloquer si l'entropie est insuffisante
        generate_random_key(file);
        
        // Note: En cas d'erreur, generate_random_key() appelle error_w()
        // qui termine le programme, donc on ne revient ici qu'en cas de succès
    }
    
    // MODE 2: Clé paramétrisée
    // Si is_key_provided == 1, la clé est déjà dans file->taille_key
    // (parsée par parse_key_from_string() appelée depuis main.c)
    // On n'a rien à faire, juste l'afficher ci-dessous

    // Affichage de la clé (commun aux deux modes)
    // Affiche au format: "key_value: 0A1B2C3D4E5F..."
    display_key(file);
}
