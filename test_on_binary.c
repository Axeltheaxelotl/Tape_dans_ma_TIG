/* ************************************************************************** */
/*                                                                            */
/*   test_on_binary.c                                                         */
/*                                                                            */
/*   Test de compression RLE sur un VRAI binaire ELF                         */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

/* ========================================================================== */
/*                          RLE                                               */
/* ========================================================================== */

size_t rle_compress(const unsigned char *input, size_t input_size,
                     unsigned char *output, size_t output_max)
{
    size_t in_pos = 0, out_pos = 0;
    
    while (in_pos < input_size)
    {
        unsigned char current = input[in_pos];
        size_t count = 1;
        
        while (in_pos + count < input_size &&
               input[in_pos + count] == current &&
               count < 127)
            count++;
        
        if (count >= 3 || (count >= 2 && current >= 128))
        {
            if (out_pos + 2 > output_max) return 0;
            output[out_pos++] = (unsigned char)(count | 0x80);
            output[out_pos++] = current;
            in_pos += count;
        }
        else
        {
            while (count > 0)
            {
                if (out_pos >= output_max) return 0;
                
                if (current & 0x80)
                {
                    if (out_pos + 2 > output_max) return 0;
                    output[out_pos++] = 0x81;
                    output[out_pos++] = current;
                }
                else
                    output[out_pos++] = current;
                
                in_pos++;
                count--;
                if (count > 0 && in_pos < input_size)
                    current = input[in_pos];
            }
        }
    }
    
    return out_pos;
}

/* ========================================================================== */
/*                          MAIN                                              */
/* ========================================================================== */

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s <binary_file>\n", argv[0]);
        printf("Exemple: %s ./test_binary\n", argv[0]);
        return 1;
    }
    
    // Ouvrir le fichier
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }
    
    // Obtenir la taille
    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        perror("fstat");
        close(fd);
        return 1;
    }
    
    size_t file_size = st.st_size;
    
    // Lire le fichier
    unsigned char *file_data = malloc(file_size);
    if (read(fd, file_data, file_size) != (ssize_t)file_size)
    {
        perror("read");
        free(file_data);
        close(fd);
        return 1;
    }
    close(fd);
    
    // Compresser
    unsigned char *compressed = malloc(file_size * 2);
    size_t compressed_size = rle_compress(file_data, file_size, compressed, file_size * 2);
    
    // Afficher les résultats
    printf("\n");
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║   🗜️  COMPRESSION RLE SUR BINAIRE ELF 🗜️            ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");
    printf("\n");
    printf("📁 Fichier: %s\n", argv[1]);
    printf("\n");
    printf("📊 RÉSULTATS:\n");
    printf("   Taille originale:  %zu bytes\n", file_size);
    printf("   Taille compressée: %zu bytes\n", compressed_size);
    printf("\n");
    
    if (compressed_size < file_size)
    {
        double ratio = 100.0 * (1.0 - (double)compressed_size / file_size);
        printf("   ✅ GAIN: %.1f%%\n", ratio);
        printf("   💾 Économie: %zu bytes\n", file_size - compressed_size);
    }
    else
    {
        printf("   ⚠️  Pas de gain (binaire non compressible)\n");
    }
    
    printf("\n");
    printf("╔══════════════════════════════════════════════════════╗\n");
    printf("║   La compression RLE est efficace sur les binaires  ║\n");
    printf("║   avec beaucoup de padding et d'alignement!         ║\n");
    printf("╚══════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    free(file_data);
    free(compressed);
    
    return 0;
}
