/* ************************************************************************** */
/*                                                                            */
/*   test_rle.c                                                               */
/*                                                                            */
/*   Programme de test pour l'algorithme RLE                                 */
/*   Compile avec: gcc -o test_rle test_rle.c rle_compress.c                 */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Déclarations des fonctions RLE */
size_t rle_compress(const unsigned char *input, size_t input_size,
                     unsigned char *output, size_t output_max);
size_t rle_decompress(const unsigned char *input, size_t input_size,
                       unsigned char *output, size_t output_max);
int rle_is_worth_it(size_t original_size, size_t compressed_size);

/* ========================================================================== */
/*                          TESTS                                             */
/* ========================================================================== */

void print_bytes(const unsigned char *data, size_t size, const char *label)
{
    printf("%s (%zu bytes): ", label, size);
    for (size_t i = 0; i < size && i < 50; i++)
        printf("%02X ", data[i]);
    if (size > 50)
        printf("...");
    printf("\n");
}

void test_simple(void)
{
    printf("\n=== TEST 1: Données simples avec répétitions ===\n");
    
    unsigned char input[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x42};
    unsigned char compressed[100];
    unsigned char decompressed[100];
    
    size_t compressed_size = rle_compress(input, sizeof(input), compressed, 100);
    print_bytes(input, sizeof(input), "Original");
    print_bytes(compressed, compressed_size, "Compressé");
    
    size_t decompressed_size = rle_decompress(compressed, compressed_size, decompressed, 100);
    print_bytes(decompressed, decompressed_size, "Décompressé");
    
    if (decompressed_size == sizeof(input) && memcmp(input, decompressed, sizeof(input)) == 0)
        printf("✅ TEST 1 RÉUSSI\n");
    else
        printf("❌ TEST 1 ÉCHOUÉ\n");
}

void test_padding(void)
{
    printf("\n=== TEST 2: Simulation padding (.bss) ===\n");
    
    unsigned char input[512];
    memset(input, 0x00, 512);  // 512 zéros
    
    unsigned char compressed[1024];
    unsigned char decompressed[512];
    
    size_t compressed_size = rle_compress(input, 512, compressed, 1024);
    
    printf("Original: 512 bytes (tous 0x00)\n");
    printf("Compressé: %zu bytes\n", compressed_size);
    printf("Ratio: %.1f%%\n", 100.0 * compressed_size / 512);
    
    size_t decompressed_size = rle_decompress(compressed, compressed_size, decompressed, 512);
    
    if (decompressed_size == 512 && memcmp(input, decompressed, 512) == 0)
        printf("✅ TEST 2 RÉUSSI (gain: %.1f%%)\n", 100.0 * (1.0 - (double)compressed_size / 512));
    else
        printf("❌ TEST 2 ÉCHOUÉ\n");
}

void test_code(void)
{
    printf("\n=== TEST 3: Simulation code avec NOPs ===\n");
    
    unsigned char input[64];
    /* Simuler du code avec des NOPs (0x90) pour alignement */
    memset(input, 0x90, 8);       // 8 NOPs
    input[8] = 0x48; input[9] = 0x89; input[10] = 0xE5;  // mov rbp, rsp
    memset(input + 11, 0x90, 5);  // 5 NOPs
    input[16] = 0xC3;              // ret
    memset(input + 17, 0x00, 47);  // padding
    
    unsigned char compressed[128];
    unsigned char decompressed[64];
    
    size_t compressed_size = rle_compress(input, 64, compressed, 128);
    
    print_bytes(input, 64, "Original");
    print_bytes(compressed, compressed_size, "Compressé");
    
    printf("Ratio: %.1f%%\n", 100.0 * compressed_size / 64);
    
    size_t decompressed_size = rle_decompress(compressed, compressed_size, decompressed, 64);
    
    if (decompressed_size == 64 && memcmp(input, decompressed, 64) == 0)
        printf("✅ TEST 3 RÉUSSI\n");
    else
        printf("❌ TEST 3 ÉCHOUÉ\n");
}

void test_worst_case(void)
{
    printf("\n=== TEST 4: Worst case (données aléatoires) ===\n");
    
    unsigned char input[100];
    for (size_t i = 0; i < 100; i++)
        input[i] = i;  // Aucune répétition
    
    unsigned char compressed[200];
    unsigned char decompressed[100];
    
    size_t compressed_size = rle_compress(input, 100, compressed, 200);
    
    printf("Original: 100 bytes (aucune répétition)\n");
    printf("Compressé: %zu bytes\n", compressed_size);
    
    if (rle_is_worth_it(100, compressed_size))
        printf("⚠️  Compression rentable (surprenant!)\n");
    else
        printf("✅ Compression non rentable (attendu)\n");
    
    size_t decompressed_size = rle_decompress(compressed, compressed_size, decompressed, 100);
    
    if (decompressed_size == 100 && memcmp(input, decompressed, 100) == 0)
        printf("✅ TEST 4 RÉUSSI\n");
    else
        printf("❌ TEST 4 ÉCHOUÉ\n");
}

int main(void)
{
    printf("╔════════════════════════════════════════════╗\n");
    printf("║   TEST COMPRESSION RLE POUR WOODY         ║\n");
    printf("╚════════════════════════════════════════════╝\n");
    
    test_simple();
    test_padding();
    test_code();
    test_worst_case();
    
    printf("\n✅ Tests terminés!\n");
    return 0;
}
