#!/bin/bash
# Script pour tester le support PE de woody_woodpacker

set -e

echo "=== Test PE Support ==="
echo ""

# Créer un binaire PE simple avec un compilateur C embarqué
cat > /tmp/test_pe_simple.c << 'EOF'
#include <stdio.h>

int main(int argc, char **argv) {
    printf("Simple PE test program!\n");
    printf("Arguments: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("  argv[%d] = %s\n", i, argv[i]);
    }
    return 0;
}
EOF

echo "Compilation d'un PE de test avec TCC (Tiny C Compiler)..."

# Vérifier si tcc est installé
if ! command -v tcc &> /dev/null; then
    echo "❌ TCC n'est pas installé."
    echo "Pour tester les PE, installez MinGW:"
    echo "  brew install mingw-w64   (sur macOS)"
    echo "  apt install mingw-w64    (sur Linux)"
    echo ""
    echo "Ou utilisez Docker:"
    echo "  docker-compose run --rm woody-test-pe"
    exit 1
fi

# Compiler avec TCC
tcc -o test_pe.exe /tmp/test_pe_simple.c

echo "✅ PE compilé : test_pe.exe"
echo ""

# Test de woody_woodpacker
echo "Exécution de woody_woodpacker sur le PE..."
./woody_woodpacker test_pe.exe

echo ""
echo "✅ woody créé"
echo ""

# Vérifier si wine est installé
if command -v wine64 &> /dev/null; then
    echo "Test d'exécution avec Wine..."
    wine64 woody.exe
else
    echo "⚠️  Wine n'est pas installé. Impossible de tester l'exécution."
    echo "Le fichier woody a été créé mais n'a pas pu être testé."
fi

echo ""
echo "=== Test terminé ==="
