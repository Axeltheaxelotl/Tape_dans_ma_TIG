#!/bin/bash

# Script pour compiler les binaires de test

set -e

echo "Compilation des binaires de test..."

# Compiler ELF 64-bit
echo "→ Compilation test_elf64..."
gcc -o test_elf64 test_elf.c -m64 -O0
echo "  ✓ test_elf64 créé"

# Compiler ELF 32-bit (si supporté)
echo "→ Compilation test_elf32..."
if gcc -o test_elf32 test_elf.c -m32 -O0 2>/dev/null; then
    echo "  ✓ test_elf32 créé"
else
    echo "  ⚠ test_elf32 skippé (support 32-bit non disponible)"
fi

# Compiler PE avec MinGW (si disponible)
echo "→ Compilation test_pe.exe..."
if command -v x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
    x86_64-w64-mingw32-gcc -o test_pe.exe test_pe.c -O0
    echo "  ✓ test_pe.exe créé"
elif command -v i686-w64-mingw32-gcc >/dev/null 2>&1; then
    i686-w64-mingw32-gcc -o test_pe.exe test_pe.c -O0
    echo "  ✓ test_pe.exe créé (32-bit)"
else
    echo "  ⚠ test_pe.exe skippé (MinGW non installé)"
fi

echo ""
echo "Binaires de test compilés avec succès!"
ls -lh test_elf* test_pe* 2>/dev/null || true
