#ifndef PH_H
#define PH_H

#include "../../includes/woody.h"

/*
PE Portable Executable le truc utiliser pour le windob
*/


#define IMG_DOS_SIGN 0x5A4D // verifie si le dos->emagic == Ox5A4D qui est la signtaure du debut de PE
#define IMG_NT_SIGN 0x00004550 // apres e_lfanew verifie si *(uint32_t*)(base + e_lfanew) == IMG_NT_SIGN
#define IMG



/*
    le vieux truc qui sert a que dall
    on garde juste le e_magic pour savoir si c bien un MZ (Ox5A4D) ducoup bas c pas PE
    e_lfnew numero qui dit ou se trouve le vrai pe headerbn                                                     
*/
typedef struct s_dos_header
{
    uint16_t e_magic;
    char padding[58];
    int32_t e_lfanew;
} t_dos_header;




typedef struct s_coff_header
{
    uint16_t Machine; //0x8664 = x64, 0x014C = x86 , en gros verifie si c est bien 0x8664
    uint16_t NumberOfSections; // nbr de sections, pour parcourire le putain de tableau de sections
    uint32_t TimeDateStamp; // c de la merde balek c pour la date de compile mais azy je la garde
    uint32_t PointerToSymbolTable; // tjr 0 ca sert plus a rien
    uint32_t NumberOfSymbols; // la meme choses ca sert plus a rien tjr 0
}; t_coof_header



int le_pasing_magique(const char *filename, t_pe_file *pe);
