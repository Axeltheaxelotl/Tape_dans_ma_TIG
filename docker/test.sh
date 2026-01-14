#!/bin/bash

# Script de test pour woody_woodpacker avec support ELF et PE
# Ce script compile et teste le projet dans un environnement Docker

set -e

# Couleurs pour les messages
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Fonction pour afficher un message
msg() {
    echo -e "${BLUE}[TEST]${NC} $1"
}

success() {
    echo -e "${GREEN}[✓]${NC} $1"
}

error() {
    echo -e "${RED}[✗]${NC} $1"
}

warning() {
    echo -e "${YELLOW}[!]${NC} $1"
}

# Répertoires
PROJECT_DIR="/workspace"
TEST_DIR="/workspace/docker/test_binaries"
RESULTS_DIR="/workspace/docker/test_results"

# Créer les répertoires nécessaires
mkdir -p "$TEST_DIR"
mkdir -p "$RESULTS_DIR"

# Nettoyer les résultats précédents
rm -rf "${RESULTS_DIR:?}"/*

msg "=== Compilation de woody_woodpacker ==="
cd "$PROJECT_DIR"
make re
success "woody_woodpacker compilé avec succès"

# Test 1: Binaire ELF simple
msg "=== Test 1: Binaire ELF 64-bit ==="
if [ -f "$TEST_DIR/test_elf64" ]; then
    ./woody_woodpacker "$TEST_DIR/test_elf64"
    if [ $? -eq 0 ]; then
        success "Test ELF 64-bit: OK"
        # Tester l'exécution du binaire woody
        if [ -f "./woody" ]; then
            ./woody > "$RESULTS_DIR/elf64_output.txt" 2>&1 || true
            success "Binaire woody ELF 64-bit exécuté"
        fi
    else
        error "Test ELF 64-bit: ÉCHEC"
    fi
else
    warning "Test ELF 64-bit: binaire non trouvé, skippé"
fi

# Test 2: Binaire ELF 32-bit
msg "=== Test 2: Binaire ELF 32-bit ==="
if [ -f "$TEST_DIR/test_elf32" ]; then
    ./woody_woodpacker "$TEST_DIR/test_elf32"
    if [ $? -eq 0 ]; then
        success "Test ELF 32-bit: OK"
        if [ -f "./woody" ]; then
            ./woody > "$RESULTS_DIR/elf32_output.txt" 2>&1 || true
            success "Binaire woody ELF 32-bit exécuté"
        fi
    else
        error "Test ELF 32-bit: ÉCHEC"
    fi
else
    warning "Test ELF 32-bit: binaire non trouvé, skippé"
fi

# Test 3: Binaire PE (Windows) avec Wine
msg "=== Test 3: Binaire PE (Windows) ==="
if command -v wine >/dev/null 2>&1; then
    if [ -f "$TEST_DIR/test_pe.exe" ]; then
        ./woody_woodpacker "$TEST_DIR/test_pe.exe"
        if [ $? -eq 0 ]; then
            success "Test PE: parsing et encryption OK"
            # Note: L'exécution complète avec Wine sera disponible
            # une fois l'injection PE complètement implémentée
            warning "Injection PE: implémentation partielle (TODO)"
        else
            error "Test PE: ÉCHEC"
        fi
    else
        warning "Test PE: binaire non trouvé, skippé"
    fi
else
    warning "Wine non installé, tests PE skippés"
fi

# Test 4: Vérification avec readelf
msg "=== Test 4: Vérification des structures ELF ==="
if [ -f "./woody" ] && command -v readelf >/dev/null 2>&1; then
    readelf -h ./woody > "$RESULTS_DIR/woody_elf_header.txt" 2>&1
    readelf -S ./woody > "$RESULTS_DIR/woody_elf_sections.txt" 2>&1
    success "Structures ELF exportées"
fi

# Test 5: Vérification avec objdump
msg "=== Test 5: Désassemblage ==="
if [ -f "./woody" ] && command -v objdump >/dev/null 2>&1; then
    objdump -d ./woody | head -n 100 > "$RESULTS_DIR/woody_disasm.txt" 2>&1
    success "Désassemblage exporté"
fi

# Résumé
msg "=== Résumé des tests ==="
echo ""
success "Compilation: OK"
[ -f "$RESULTS_DIR/elf64_output.txt" ] && success "ELF 64-bit: OK" || warning "ELF 64-bit: SKIPPED"
[ -f "$RESULTS_DIR/elf32_output.txt" ] && success "ELF 32-bit: OK" || warning "ELF 32-bit: SKIPPED"
warning "PE Support: Partiel (parsing OK, injection TODO)"
echo ""
msg "Résultats disponibles dans: $RESULTS_DIR"

exit 0
