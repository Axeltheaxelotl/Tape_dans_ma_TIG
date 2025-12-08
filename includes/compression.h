#ifndef COMPRESSION_H
#define COMPRESSION_H

#include "woody.h"

//les types de compression
typedef enum s_type_de_compression
{
    COMP_NONE = 0,
	COMP_RLE = 1,
	COMP_LZ77 = 2,
	COMP_DEFLATE = 3,
	COMP_LZMA = 4
} t_type_de_compression;


//stockage result compression
typedef struct s_comp_result
{
    void  *compressed_data;
    size_t taille_compressed;
    size_t taille_original;
    t_type_de_compression algo;
    int is_worth_it;
} t_comp_result;



//algo de compression RLE
//Format: [count|0x80][value] pour répétitions, [value] pour literals
size_t compression(const unsigned char *input, size_t taille_input, unsigned char *output, size_t max_output);
size_t decompression(const unsigned char *input, size_t taille_input, unsigned char *output, size_t max_output);
//test pour savoir si c rentable
int rle_is_worth_it(size_t taille_original, size_t taille_compressed);


//stubs et decompression en assembly (je precise appele dans le payload)
extern size_t rle_decompress_stub_32(const unsigned char *input, size_t taille_input, unsigned char *output, size_t max_output);
extern size_t rle_decompress_stub_64(const unsigned char *input, size_t taille_input, unsigned char *output, size_t max_output);

#endif