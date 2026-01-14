#!/bin/bash
# Script de test complet : ELF + PE

set -e

echo "🪵 Woody Woodpacker - Complete Test Suite"
echo "=========================================="
echo ""

GREEN='\033[0;32m'
RED='\033[0;31m'
BLUE='\033[0;34m'
NC='\033[0m'

# 1. Compiler woody_woodpacker
echo -e "${BLUE}[1/3]${NC} Building woody_woodpacker..."
make re > /dev/null 2>&1
echo -e "${GREEN}✓${NC} Build successful"
echo ""

# 2. Tests ELF
echo -e "${BLUE}[2/3]${NC} Running ELF tests..."
if [ -f "docker/test.sh" ]; then
    bash docker/test.sh
else
    echo -e "${RED}✗${NC} docker/test.sh not found"
fi
echo ""

# 3. Tests PE
echo -e "${BLUE}[3/3]${NC} Running PE tests..."
if [ -f "docker/test_pe.sh" ]; then
    bash docker/test_pe.sh
else
    echo -e "${RED}✗${NC} docker/test_pe.sh not found"
fi

echo ""
echo "=========================================="
echo -e "${GREEN}✓ All tests completed!${NC}"
echo "=========================================="
echo ""
echo "Results available in: docker/test_results/"
