#include "../../includes/woody.h"

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

        //compte les repetitions max 127 pour garder les bite 7
        while(in_pos + count < taille_input && input[in_pos + count] == courant && count < 127)
        {
            count++;
        }
        //attention la ca decide si RLE est rentable
        if (count >= 3 || (count >= 2 && courant >= 128))
        {
            //mode RLE : [count|0x8O][byte]
            if (out_pos + 2 > max_output)
                return 0;
            output[out_pos++] = (unsigned char)(count | 0x80);
            output[out_pos++] = courant;
            in_pos += count;
        }
        else{
            //evite rle si pas rentable - traiter les bytes individuellement
            size_t bytes_to_process = count;
            size_t i;
            
            for (i = 0; i < bytes_to_process; i++)
            {
                if (out_pos >= max_output)
                    return 0;
                
                unsigned char current_byte = input[in_pos];
                
                //verifie si byte a bit 7 set
                if (current_byte & 0x80)
                {
                    //forcer rle count = 1 pour bytes >= 128
                    if (out_pos + 2 > max_output)
                        return 0;
                    output[out_pos++] = 0x81;  // count=1 avec bit 7
                    output[out_pos++] = current_byte;
                }
                else
                {
                    //copie literal simple
                    output[out_pos++] = current_byte;
                }
                in_pos++;
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
            //lire count et valeur
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
        else {
            //copier directement 
            if (out_pos >= max_output)
                return 0;
            output[out_pos++] = byte;
        }
    }
    return (out_pos);
}





int rle_is_worth_it(size_t taille_original, size_t taille_compressed)
{
    /* Compression rentable si on gagne au moins 10% */
    return (taille_compressed < taille_original * 90 / 100);
}