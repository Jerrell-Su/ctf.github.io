#!/bin/bash

# Test all CTF challenges
# Checks: compilation, binary exists, checksec output

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo "Testing all CTF challenges..."
echo "=============================="

# Check for required tools
if ! command -v checksec &> /dev/null; then
    echo -e "${YELLOW}Warning: checksec not found. Install with: apt-get install checksec${NC}"
    echo "Will skip protection checks."
    HAS_CHECKSEC=0
else
    HAS_CHECKSEC=1
fi

TOTAL=0
SUCCESS=0
FAILED=0

# Find all challenge directories
for dir in */; do
    # Skip if not a challenge directory
    if [ ! -f "$dir/Makefile" ]; then
        continue
    fi

    TOTAL=$((TOTAL + 1))
    challenge_name=$(basename "$dir")
    challenge_path="$dir/challenge"

    echo ""
    echo -e "${YELLOW}Testing $challenge_name...${NC}"

    # Check if binary exists
    if [ ! -f "$challenge_path" ]; then
        echo -e "${RED}✗ Binary not found: $challenge_path${NC}"
        echo -e "${BLUE}  Run: cd $dir && make${NC}"
        FAILED=$((FAILED + 1))
        continue
    fi

    # Check if binary is executable
    if [ ! -x "$challenge_path" ]; then
        echo -e "${RED}✗ Binary not executable${NC}"
        FAILED=$((FAILED + 1))
        continue
    fi

    # Check architecture
    file_output=$(file "$challenge_path")
    if echo "$file_output" | grep -q "x86-64"; then
        echo -e "${GREEN}✓ Architecture: x86-64${NC}"
    else
        echo -e "${RED}✗ Unexpected architecture: $file_output${NC}"
        FAILED=$((FAILED + 1))
        continue
    fi

    # Run checksec if available
    if [ $HAS_CHECKSEC -eq 1 ]; then
        echo -e "${BLUE}  Protections:${NC}"
        checksec --file="$challenge_path" | grep -E "(RELRO|STACK CANARY|NX|PIE)" | sed 's/^/    /'
    fi

    # Verify solution file exists
    if [ -f "$dir/SOLUTION.md" ]; then
        echo -e "${GREEN}✓ Solution documentation exists${NC}"
    else
        echo -e "${YELLOW}⚠ No SOLUTION.md found${NC}"
    fi

    # Verify Dockerfile exists
    if [ -f "$dir/Dockerfile" ]; then
        echo -e "${GREEN}✓ Dockerfile exists${NC}"
    else
        echo -e "${YELLOW}⚠ No Dockerfile found${NC}"
    fi

    SUCCESS=$((SUCCESS + 1))
    echo -e "${GREEN}✓ $challenge_name passed all tests${NC}"
done

echo ""
echo "=============================="
echo "Test Summary:"
echo "  Total:   $TOTAL"
echo -e "  ${GREEN}Success: $SUCCESS${NC}"
echo -e "  ${RED}Failed:  $FAILED${NC}"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All challenges passed tests!${NC}"
    exit 0
else
    echo -e "${RED}Some challenges failed tests${NC}"
    exit 1
fi
