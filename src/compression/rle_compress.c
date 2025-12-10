#include "../../includes/woody.h"

/*
** Format RLE amélioré :
** - [count|0x80][byte]                    : Répétition (count ≥ 2)
** - [count][byte1][byte2]...[byteN]       : Literal run (count ≤ 127 bytes non répétés)
**
** Ceci évite l'explosion de taille sur des données denses.
*/
size_t compression(const unsigned char *input, size_t taille_input, unsigned char *output, size_t max_output)
{
    size_t in_pos;
    size_t out_pos;
    unsigned char courant;
    size_t count;

    in_pos = 0;
    out_pos = 0;

    if (!input || !output || taille_input == 0)
        return 0;

    while(in_pos < taille_input)
    {
        courant = input[in_pos];
        count = 1;

        // Compte les répétitions (max 127)
        while(in_pos + count < taille_input && input[in_pos + count] == courant && count < 127)
        {
            count++;
        }
        
        // Si répétition rentable (≥3 bytes identiques), encoder en RLE
        if (count >= 3)
        {
            if (out_pos + 2 > max_output)
                return 0;
            output[out_pos++] = (unsigned char)(count | 0x80);  // Bit 7 = RLE mode
            output[out_pos++] = courant;
            in_pos += count;
        }
        else
        {
            // Pas de répétition : collecter un "literal run"
            size_t literal_start = in_pos;
            size_t literal_count = 0;
            
            // Collecter les bytes non répétés (max 127)
            while (in_pos < taille_input && literal_count < 127)
            {
                // Vérifier si une répétition commence
                size_t lookahead = 1;
                while (in_pos + lookahead < taille_input && 
                       input[in_pos + lookahead] == input[in_pos] && 
                       lookahead < 3)
                {
                    lookahead++;
                }
                
                // Si répétition ≥3, arrêter le literal run
                if (lookahead >= 3)
                    break;
                
                literal_count++;
                in_pos++;
            }
            
            // Encoder le literal run : [count][byte1][byte2]...[byteN]
            if (out_pos + 1 + literal_count > max_output)
                return 0;
            
            output[out_pos++] = (unsigned char)literal_count;  // Sans bit 7
            for (size_t i = 0; i < literal_count; i++)
            {
                output[out_pos++] = input[literal_start + i];
            }
        }
    }
    return (out_pos);
}






//decompresser le rle
size_t decompression(const unsigned char *input, size_t taille_input, unsigned char *output, size_t max_output)
{
    size_t in_pos;
    size_t out_pos;
    unsigned char byte;
    size_t count;
    unsigned char valeur;
    size_t i;

    in_pos = 0;
    out_pos = 0;

    if(!input || !output || taille_input == 0)
    {
        return 0;
    }

    while(in_pos < taille_input)
    {
        byte = input[in_pos++];
        
        if (byte & 0x80)
        {
            // Mode RLE : répétition
            count = byte & 0x7F;

            if (in_pos >= taille_input)
            {
                return 0;
            }
            valeur = input[in_pos++];

            //verifie espace dispo
            if (out_pos + count > max_output)
                return 0;
            //repeter le byte
            i = 0;
            while(i < count)
            {
                output[out_pos++] = valeur;
                i++;
            }
        }
        else
        {
            // Mode Literal : copier 'count' bytes
            count = byte;
            
            if (in_pos + count > taille_input)
                return 0;
            if (out_pos + count > max_output)
                return 0;
            
            // Copier tous les bytes du literal run
            for (i = 0; i < count; i++)
            {
                output[out_pos++] = input[in_pos++];
            }
        }
    }
    return (out_pos);
}





int rle_is_worth_it(size_t taille_original, size_t taille_compressed)
{
   // compression rentable si taille compresser inferieur
    return (taille_compressed < taille_original);
}