#!/bin/bash

# Script pour extraire le bytecode d'un stub ASM et générer le code C

if [ $# -ne 2 ]; then
    echo "Usage: $0 <stub.o> <output_name>"
    echo "Example: $0 obj/asm/decrypt_decompress_64.o DECRYPT_DECOMPRESS_64"
    exit 1
fi

STUB_OBJ="$1"
OUTPUT_NAME="$2"

if [ ! -f "$STUB_OBJ" ]; then
    echo "Error: File $STUB_OBJ not found"
    exit 1
fi

# Extraire la section .text en binaire
objcopy -O binary -j .text "$STUB_OBJ" /tmp/stub.bin

# Calculer la taille
SIZE=$(stat -c%s /tmp/stub.bin)

# Générer le tableau C sans les 0x00 padding à la fin
echo "// Stub: $OUTPUT_NAME ($SIZE bytes)"
echo "static const unsigned char ${OUTPUT_NAME}[] = {"

# Convertir en hex, enlever trailing 0x00, puis reformater
xxd -p -c 16 /tmp/stub.bin | \
    while read line; do
        # Convertir la ligne hex en bytes séparés par virgule
        echo "$line" | sed 's/\(..\)/0x\1, /g'
    done | tr '\n' ' ' | sed 's/, $//' | \
    fold -w 80 | sed 's/^/    /' | sed 's/ *$//' | sed 's/, *$//'

echo ""
echo "};"
echo "#define ${OUTPUT_NAME}_SIZE $SIZE"
echo ""

# Cleanup
rm -f /tmp/stub.bin
