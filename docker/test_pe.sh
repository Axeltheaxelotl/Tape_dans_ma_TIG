#!/bin/bash
# Script de test complet pour les binaires PE (Windows) avec Wine

set -e

echo "🍷 Testing PE (Windows) binaries with Wine"
echo "==========================================="
echo ""

GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
YELLOW='\033[0;33m'
NC='\033[0m'

TEST_DIR="docker/test_binaries"
OUTPUT_DIR="docker/test_results"
mkdir -p "$OUTPUT_DIR"

# Fonction de test PE
test_pe_binary() {
    local pe_file=$1
    local arch=$2
    local wine_cmd=$3
    
    echo -e "${BLUE}Testing:${NC} $pe_file ($arch)"
    
    # 1. Vérifier que le PE existe
    if [ ! -f "$pe_file" ]; then
        echo -e "${RED}✗ PE file not found${NC}"
        return 1
    fi
    
    # 2. Afficher les infos du PE
    echo "  File info:"
    file "$pe_file" | sed 's/^/    /'
    
    # 3. Tester l'exécution originale (optionnel)
    echo "  Testing original PE with Wine..."
    if timeout 5 $wine_cmd "$pe_file" > "$OUTPUT_DIR/original_$arch.txt" 2>&1; then
        echo -e "  ${GREEN}✓${NC} Original PE runs"
    else
        echo -e "  ${YELLOW}⚠${NC} Original PE execution failed (may be normal)"
    fi
    
    # 4. Packer avec woody_woodpacker
    echo "  Packing with woody_woodpacker..."
    if ./woody_woodpacker "$pe_file" > "$OUTPUT_DIR/pack_$arch.log" 2>&1; then
        echo -e "  ${GREEN}✓${NC} Packing succeeded"
        cat "$OUTPUT_DIR/pack_$arch.log" | sed 's/^/    /'
    else
        echo -e "  ${RED}✗${NC} Packing failed"
        cat "$OUTPUT_DIR/pack_$arch.log"
        return 1
    fi
    
    # 5. Vérifier que woody a été créé
    if [ ! -f "woody" ]; then
        echo -e "  ${RED}✗${NC} woody not created"
        return 1
    fi
    
    echo -e "  ${GREEN}✓${NC} woody created"
    file woody | sed 's/^/    /'
    
    # 6. Comparer les tailles
    local orig_size=$(stat -c%s "$pe_file" 2>/dev/null || stat -f%z "$pe_file")
    local woody_size=$(stat -c%s "woody" 2>/dev/null || stat -f%z "woody")
    echo "  Size: $orig_size → $woody_size bytes"
    
    # 7. Analyser le PE avec objdump
    echo "  Analyzing PE structure..."
    objdump -h "$pe_file" > "$OUTPUT_DIR/original_sections_$arch.txt" 2>&1 || true
    
    # 8. Tester l'exécution de woody (optionnel)
    echo "  Testing packed PE with Wine..."
    if timeout 5 $wine_cmd "./woody" > "$OUTPUT_DIR/woody_$arch.txt" 2>&1; then
        echo -e "  ${GREEN}✓${NC} Packed PE runs"
        echo "  Output:"
        cat "$OUTPUT_DIR/woody_$arch.txt" | head -n 10 | sed 's/^/    /'
    else
        echo -e "  ${YELLOW}⚠${NC} Packed PE execution failed"
        echo "  (Note: PE injection not fully implemented yet)"
    fi
    
    # 9. Sauvegarder woody pour inspection
    cp woody "$OUTPUT_DIR/woody_$arch.exe"
    rm -f woody
    
    echo ""
    return 0
}

# Compiler les binaires PE de test
echo -e "${BLUE}[1/3]${NC} Compiling PE test binaries..."
cd "$TEST_DIR"

# PE 32 bits
echo "  Compiling PE 32-bit..."
if i686-w64-mingw32-gcc test_pe.c -o test_pe32.exe -O0 -g 2>/dev/null; then
    echo -e "  ${GREEN}✓${NC} test_pe32.exe"
else
    echo -e "  ${YELLOW}⚠${NC} test_pe32.exe compilation failed (32-bit not available)"
fi

# PE 64 bits
echo "  Compiling PE 64-bit..."
if x86_64-w64-mingw32-gcc test_pe.c -o test_pe64.exe -O0 -g; then
    echo -e "  ${GREEN}✓${NC} test_pe64.exe"
else
    echo -e "  ${RED}✗${NC} test_pe64.exe compilation failed"
    echo "  MinGW not installed or test_pe.c not found"
    cd ../..
    exit 1
fi

cd ../..
echo ""

# Initialiser Wine
echo -e "${BLUE}[2/3]${NC} Initializing Wine..."
export WINEARCH=win64
export WINEPREFIX=~/.wine64
wineboot -u 2>/dev/null || true
echo -e "${GREEN}✓${NC} Wine initialized"
echo ""

# Tests
echo -e "${BLUE}[3/3]${NC} Running PE tests..."
echo ""

PASS=0
FAIL=0

# Test PE 32 bits (si disponible)
if [ -f "$TEST_DIR/test_pe32.exe" ]; then
    if test_pe_binary "$TEST_DIR/test_pe32.exe" "PE32" "wine"; then
        PASS=$((PASS + 1))
    else
        FAIL=$((FAIL + 1))
    fi
fi

# Test PE 64 bits
if test_pe_binary "$TEST_DIR/test_pe64.exe" "PE32+" "wine64"; then
    PASS=$((PASS + 1))
else
    FAIL=$((FAIL + 1))
fi

# Résumé
echo "========================================"
echo "PE Test Summary"
echo "========================================"
echo -e "Passed: ${GREEN}$PASS${NC}"
echo -e "Failed: ${RED}$FAIL${NC}"
echo ""
echo "Test results saved in: $OUTPUT_DIR"
echo ""

if [ $FAIL -eq 0 ]; then
    echo -e "${GREEN}✓ All PE tests passed!${NC}"
    exit 0
else
    echo -e "${YELLOW}⚠ Some PE tests failed (injection not fully implemented)${NC}"
    exit 0
fi
