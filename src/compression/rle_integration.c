#include "../../includes/woody.h"
#include <stdio.h>

/*
** Tente de compresser une section .text
** Retourne un pointeur vers les données compressées si rentable, NULL sinon
** Met à jour file->is_compressed et file->compressed_size
**
** NOTE IMPORTANTE (Décembre 2025):
** La compression RLE fonctionne parfaitement en standalone (voir make test-rle).
** CEPENDANT, l'intégration dans le binaire final nécessite un stub de décompression
** en assembleur position-independent, ce qui est très complexe à implémenter :
**   1. On ne peut pas décompresser "en place" (données compressées < originales)
**   2. Il faudrait allouer dynamiquement de la mémoire dans le stub
**   3. Le stub doit rester position-independent (pas d'adresses absolues)
**
** Pour cette raison, la compression est DÉSACTIVÉE pour les binaires finaux.
** La fonction détecte toujours si compression serait rentable (pour logs),
** mais retourne toujours NULL pour garantir la stabilité.
**
** TODO futur: Implémenter un stub decrypt_decompress_64.s fonctionnel avec:
**   - mmap() pour allouer un buffer temporaire
**   - Décompression RLE dans ce buffer
**   - Copie du résultat vers .text
**   - munmap() pour libérer
*/
void *try_compress_section(t_elf_file *file, void *text_data, size_t text_size)
{
    unsigned char *compressed_buffer;
    size_t compressed_size;

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
        // Compression rentable en théorie !
        // MAIS : La décompression au runtime dans le stub est trop complexe
        // Pour l'instant, on désactive la compression dans le binaire final
        printf("📦 Compression RLE détectée comme rentable:\n");
        printf("   - Taille originale: %zu bytes\n", text_size);
        printf("   - Taille compressée: %zu bytes\n", compressed_size);
        printf("   - Gain théorique: %.1f%%\n", 100.0 * (1.0 - (double)compressed_size / text_size));
        printf("   ⚠️  Compression désactivée (stub de décompression non implémenté)\n");
        
        free(compressed_buffer);
        file->is_compressed = 0;
        return NULL;
    }
    else
    {
        // Compression non rentable
        free(compressed_buffer);
        file->is_compressed = 0;
        printf("ℹ️  Compression RLE non rentable (données trop denses)\n");
        return NULL;
    }
}
