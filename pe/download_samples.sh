#!/bin/bash
# Script pour télécharger et tester des fichiers PE

set -e

echo "🔍 Téléchargement d'un fichier PE de test..."

# Créer un dossier pour les tests
mkdir -p pe_samples
cd pe_samples

# Télécharger un fichier PE tiny (très petit, bon pour les tests)
if [ ! -f tiny.exe ]; then
    echo "📥 Téléchargement de tiny.exe..."
    wget -q https://github.com/corkami/pocs/raw/master/PE/bin/tiny.exe || {
        echo "❌ Échec du téléchargement"
        echo "Vous pouvez aussi compiler vos propres PE avec MinGW:"
        echo "  sudo apt install mingw-w64"
        echo "  x86_64-w64-mingw32-gcc test.c -o test.exe"
        exit 1
    }
    echo "✅ tiny.exe téléchargé"
fi

# Télécharger un PE 64-bit normal
if [ ! -f helloworld-x64.exe ]; then
    echo "📥 Téléchargement de helloworld-x64.exe..."
    wget -q https://github.com/corkami/pocs/raw/master/PE/bin/helloworld-x64.exe || {
        echo "⚠️  Échec du téléchargement de helloworld-x64.exe"
    }
fi

# Télécharger un PE 32-bit normal
if [ ! -f helloworld.exe ]; then
    echo "📥 Téléchargement de helloworld.exe (32-bit)..."
    wget -q https://github.com/corkami/pocs/raw/master/PE/bin/helloworld.exe || {
        echo "⚠️  Échec du téléchargement de helloworld.exe"
    }
fi

cd ..

echo ""
echo "✨ Fichiers PE de test prêts!"
echo ""
echo "Pour tester:"
echo "  ./pe_test pe_samples/tiny.exe"
echo "  ./pe_test pe_samples/helloworld-x64.exe"
echo "  ./pe_test pe_samples/helloworld.exe"
echo ""
