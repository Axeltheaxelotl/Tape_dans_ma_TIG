#include "../../includes/woody.h"

/*
** Tente de compresser une section .text
** Retourne un pointeur vers les données compressées si rentable, NULL sinon
** Met à jour file->is_compressed et file->compressed_size
*/
void *try_compress_section(t_elf_file *file, void *text_data, size_t text_size)
{
    unsigned char *compressed_buffer;
    size_t compressed_size;
    void *result;

    // Allouer un buffer suffisamment grand pour la compression
    // Pire cas: la compression peut agrandir les données
    compressed_buffer = malloc(text_size * 2);
    if (!compressed_buffer)
        return NULL;

    // Compresser
    compressed_size = compression((unsigned char *)text_data, text_size, 
                                  compressed_buffer, text_size * 2);
    
    if (compressed_size == 0)
    {
        free(compressed_buffer);
        return NULL;
    }

    // Vérifier si la compression est rentable (gain ≥ 10%)
    if (rle_is_worth_it(text_size, compressed_size))
    {
        // Compression rentable !
        file->is_compressed = 1;
        file->compressed_size = compressed_size;
        file->original_size = text_size;
        
        // Réallouer au juste taille pour ne pas gaspiller
        result = malloc(compressed_size);
        if (!result)
        {
            free(compressed_buffer);
            return NULL;
        }
        ft_memcpy(result, compressed_buffer, compressed_size);
        free(compressed_buffer);
        
        // Afficher les stats
        ft_printf("📦 Compression RLE activée:\n");
        ft_printf("   - Taille originale: %zu bytes\n", text_size);
        ft_printf("   - Taille compressée: %zu bytes\n", compressed_size);
        ft_printf("   - Gain: %.1f%%\n", 100.0 * (1.0 - (double)compressed_size / text_size));
        
        return result;
    }
    else
    {
        // Compression non rentable
        free(compressed_buffer);
        file->is_compressed = 0;
        ft_printf("ℹ️  Compression RLE non rentable (données trop denses)\n");
        return NULL;
    }
}
