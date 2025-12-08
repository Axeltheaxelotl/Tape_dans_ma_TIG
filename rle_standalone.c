/* ************************************************************************** */
/*                                                                            */
/*   rle_standalone.c                                                         */
/*                                                                            */
/*   Programme de test standalone pour l'algorithme RLE                      */
/*   Teste compression C et décompression ASM                                */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

/* ========================================================================== */
/*                     PROTOTYPES ET FONCTIONS EXTERNES                       */
/* ========================================================================== */

// Fonctions de compression/décompression (depuis rle_compress.c)
size_t compression(const unsigned char *input, size_t taille_input, 
                   unsigned char *output, size_t max_output);
size_t decompression(const unsigned char *input, size_t taille_input, 
                     unsigned char *output, size_t max_output);
int rle_is_worth_it(size_t taille_original, size_t taille_compressed);

// Décompression ASM (depuis rle_decompress_64.s)
extern size_t rle_decompress_stub_64(const unsigned char *input, size_t taille_input,
                                      unsigned char *output, size_t max_output);

/* ========================================================================== */
/*                          UTILITAIRES D'AFFICHAGE                           */
/* ========================================================================== */

void print_bytes(const unsigned char *data, size_t size, const char *label)
{
    printf("  %s (%zu bytes): ", label, size);
    for (size_t i = 0; i < size && i < 40; i++)
        printf("%02X ", data[i]);
    if (size > 40)
        printf("...");
    printf("\n");
}

void print_header(const char *title)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║ %-58s ║\n", title);
    printf("╚════════════════════════════════════════════════════════════╝\n");
}

void print_result(int passed, const char *test_name)
{
    if (passed)
        printf("  ✅ %s RÉUSSI\n", test_name);
    else
        printf("  ❌ %s ÉCHOUÉ\n", test_name);
}

/* ========================================================================== */
/*                                TESTS UNITAIRES                             */
/* ========================================================================== */

int test_1_simple_repetitions(void)
{
    print_header("TEST 1: Répétitions simples");
    
    unsigned char input[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x42};
    unsigned char compressed[100];
    unsigned char decompressed_c[100];
    unsigned char decompressed_asm[100];
    
    // Compression
    size_t comp_size = compression(input, sizeof(input), compressed, 100);
    print_bytes(input, sizeof(input), "Original");
    print_bytes(compressed, comp_size, "Compressé");
    printf("  Ratio: %.1f%% (gain: %.1f%%)\n", 
           100.0 * comp_size / sizeof(input),
           100.0 * (1.0 - (double)comp_size / sizeof(input)));
    
    // Décompression C
    size_t decomp_size_c = decompression(compressed, comp_size, decompressed_c, 100);
    print_bytes(decompressed_c, decomp_size_c, "Décompressé (C)");
    
    // Décompression ASM
    size_t decomp_size_asm = rle_decompress_stub_64(compressed, comp_size, decompressed_asm, 100);
    print_bytes(decompressed_asm, decomp_size_asm, "Décompressé (ASM)");
    
    int passed = (decomp_size_c == sizeof(input) && 
                  decomp_size_asm == sizeof(input) &&
                  memcmp(input, decompressed_c, sizeof(input)) == 0 &&
                  memcmp(input, decompressed_asm, sizeof(input)) == 0);
    
    print_result(passed, "Test 1");
    return passed;
}

int test_2_padding_bss(void)
{
    print_header("TEST 2: Simulation padding .bss (512 zéros)");
    
    unsigned char input[512];
    memset(input, 0x00, 512);
    
    unsigned char compressed[1024];
    unsigned char decompressed_c[512];
    unsigned char decompressed_asm[512];
    
    size_t comp_size = compression(input, 512, compressed, 1024);
    
    printf("  Original: 512 bytes (tous 0x00)\n");
    printf("  Compressé: %zu bytes\n", comp_size);
    printf("  Ratio: %.1f%% (gain: %.1f%%)\n", 
           100.0 * comp_size / 512,
           100.0 * (1.0 - (double)comp_size / 512));
    
    size_t decomp_size_c = decompression(compressed, comp_size, decompressed_c, 512);
    size_t decomp_size_asm = rle_decompress_stub_64(compressed, comp_size, decompressed_asm, 512);
    
    int passed = (decomp_size_c == 512 && 
                  decomp_size_asm == 512 &&
                  memcmp(input, decompressed_c, 512) == 0 &&
                  memcmp(input, decompressed_asm, 512) == 0 &&
                  rle_is_worth_it(512, comp_size));
    
    print_result(passed, "Test 2");
    return passed;
}

int test_3_code_avec_nops(void)
{
    print_header("TEST 3: Simulation code avec NOPs et padding");
    
    unsigned char input[128];
    size_t pos = 0;
    
    // Simuler du code avec des NOPs (0x90) pour alignement
    memset(input + pos, 0x90, 10); pos += 10;  // 10 NOPs
    input[pos++] = 0x48; input[pos++] = 0x89; input[pos++] = 0xE5;  // mov rbp, rsp
    memset(input + pos, 0x90, 6); pos += 6;    // 6 NOPs
    input[pos++] = 0xC3;                        // ret
    memset(input + pos, 0x00, 128 - pos);       // padding
    
    unsigned char compressed[256];
    unsigned char decompressed_c[128];
    unsigned char decompressed_asm[128];
    
    size_t comp_size = compression(input, 128, compressed, 256);
    
    print_bytes(input, 30, "Original (début)");
    print_bytes(compressed, comp_size, "Compressé");
    printf("  Ratio: %.1f%% (gain: %.1f%%)\n", 
           100.0 * comp_size / 128,
           100.0 * (1.0 - (double)comp_size / 128));
    
    size_t decomp_size_c = decompression(compressed, comp_size, decompressed_c, 128);
    size_t decomp_size_asm = rle_decompress_stub_64(compressed, comp_size, decompressed_asm, 128);
    
    int passed = (decomp_size_c == 128 && 
                  decomp_size_asm == 128 &&
                  memcmp(input, decompressed_c, 128) == 0 &&
                  memcmp(input, decompressed_asm, 128) == 0);
    
    print_result(passed, "Test 3");
    return passed;
}

int test_4_worst_case(void)
{
    print_header("TEST 4: Worst case (données aléatoires)");
    
    unsigned char input[200];
    for (size_t i = 0; i < 200; i++)
        input[i] = i % 256;  // Aucune répétition consécutive
    
    unsigned char compressed[400];
    unsigned char decompressed_c[200];
    unsigned char decompressed_asm[200];
    
    size_t comp_size = compression(input, 200, compressed, 400);
    
    printf("  Original: 200 bytes (aucune répétition)\n");
    printf("  Compressé: %zu bytes\n", comp_size);
    printf("  Ratio: %.1f%%\n", 100.0 * comp_size / 200);
    
    if (rle_is_worth_it(200, comp_size))
        printf("  ⚠️  Compression rentable (surprenant!)\n");
    else
        printf("  ✓ Compression non rentable (attendu)\n");
    
    size_t decomp_size_c = decompression(compressed, comp_size, decompressed_c, 200);
    size_t decomp_size_asm = rle_decompress_stub_64(compressed, comp_size, decompressed_asm, 200);
    
    int passed = (decomp_size_c == 200 && 
                  decomp_size_asm == 200 &&
                  memcmp(input, decompressed_c, 200) == 0 &&
                  memcmp(input, decompressed_asm, 200) == 0);
    
    print_result(passed, "Test 4");
    return passed;
}

int test_5_bytes_haute_valeur(void)
{
    print_header("TEST 5: Bytes avec bit 7 set (0x80-0xFF)");
    
    // Tester les bytes >= 128 qui nécessitent un traitement spécial
    unsigned char input[] = {0xFF, 0xFF, 0xFF, 0x80, 0x80, 0x42, 0x42, 0x42, 0x42};
    unsigned char compressed[100];
    unsigned char decompressed_c[100];
    unsigned char decompressed_asm[100];
    
    size_t comp_size = compression(input, sizeof(input), compressed, 100);
    print_bytes(input, sizeof(input), "Original");
    print_bytes(compressed, comp_size, "Compressé");
    
    size_t decomp_size_c = decompression(compressed, comp_size, decompressed_c, 100);
    size_t decomp_size_asm = rle_decompress_stub_64(compressed, comp_size, decompressed_asm, 100);
    
    print_bytes(decompressed_c, decomp_size_c, "Décompressé (C)");
    print_bytes(decompressed_asm, decomp_size_asm, "Décompressé (ASM)");
    
    int passed = (decomp_size_c == sizeof(input) && 
                  decomp_size_asm == sizeof(input) &&
                  memcmp(input, decompressed_c, sizeof(input)) == 0 &&
                  memcmp(input, decompressed_asm, sizeof(input)) == 0);
    
    print_result(passed, "Test 5");
    return passed;
}

int test_6_longue_sequence(void)
{
    print_header("TEST 6: Longue séquence (> 127 répétitions)");
    
    unsigned char input[256];
    memset(input, 0xAA, 256);  // 256 fois 0xAA
    
    unsigned char compressed[512];
    unsigned char decompressed_c[256];
    unsigned char decompressed_asm[256];
    
    size_t comp_size = compression(input, 256, compressed, 512);
    
    printf("  Original: 256 bytes (tous 0xAA)\n");
    printf("  Compressé: %zu bytes\n", comp_size);
    print_bytes(compressed, comp_size, "Compressé");
    
    size_t decomp_size_c = decompression(compressed, comp_size, decompressed_c, 256);
    size_t decomp_size_asm = rle_decompress_stub_64(compressed, comp_size, decompressed_asm, 256);
    
    int passed = (decomp_size_c == 256 && 
                  decomp_size_asm == 256 &&
                  memcmp(input, decompressed_c, 256) == 0 &&
                  memcmp(input, decompressed_asm, 256) == 0);
    
    print_result(passed, "Test 6");
    return passed;
}

int test_7_cas_limites(void)
{
    print_header("TEST 7: Cas limites (buffer vide, 1 byte, etc.)");
    
    int all_passed = 1;
    
    // Test buffer vide
    unsigned char empty_comp[10];
    
    size_t size = compression(NULL, 0, empty_comp, 10);
    all_passed &= (size == 0);
    printf("  ✓ Buffer NULL: %s\n", size == 0 ? "OK" : "FAIL");
    
    // Test 1 byte
    unsigned char one_byte[1] = {0x42};
    unsigned char one_comp[10];
    unsigned char one_decomp_c[10];
    unsigned char one_decomp_asm[10];
    
    size_t comp_size = compression(one_byte, 1, one_comp, 10);
    size_t decomp_size_c = decompression(one_comp, comp_size, one_decomp_c, 10);
    size_t decomp_size_asm = rle_decompress_stub_64(one_comp, comp_size, one_decomp_asm, 10);
    
    all_passed &= (decomp_size_c == 1 && decomp_size_asm == 1 && 
                   one_decomp_c[0] == 0x42 && one_decomp_asm[0] == 0x42);
    printf("  ✓ 1 byte: %s\n", 
           (decomp_size_c == 1 && one_decomp_c[0] == 0x42) ? "OK" : "FAIL");
    
    print_result(all_passed, "Test 7");
    return all_passed;
}

/* ========================================================================== */
/*                                   MAIN                                     */
/* ========================================================================== */

int main(void)
{
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                                                            ║\n");
    printf("║       TEST COMPRESSION RLE POUR WOODY_WOODPACKER          ║\n");
    printf("║                                                            ║\n");
    printf("║  Tests de compression (C) et décompression (C + ASM)      ║\n");
    printf("║                                                            ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    
    int total = 0;
    int passed = 0;
    
    total++; if (test_1_simple_repetitions()) passed++;
    total++; if (test_2_padding_bss()) passed++;
    total++; if (test_3_code_avec_nops()) passed++;
    total++; if (test_4_worst_case()) passed++;
    total++; if (test_5_bytes_haute_valeur()) passed++;
    total++; if (test_6_longue_sequence()) passed++;
    total++; if (test_7_cas_limites()) passed++;
    
    printf("\n");
    printf("╔════════════════════════════════════════════════════════════╗\n");
    printf("║                        RÉSULTATS                           ║\n");
    printf("╠════════════════════════════════════════════════════════════╣\n");
    printf("║  Tests réussis: %d/%d                                       ║\n", passed, total);
    if (passed == total)
        printf("║  🎉 TOUS LES TESTS SONT PASSÉS ! 🎉                      ║\n");
    else
        printf("║  ⚠️  Certains tests ont échoué                           ║\n");
    printf("╚════════════════════════════════════════════════════════════╝\n");
    printf("\n");
    
    return (passed == total) ? 0 : 1;
}
