#!/bin/bash

# Build all CTF challenges
# Usage: ./build_all.sh

set -e

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "Building all CTF challenges..."
echo "=============================="

SUCCESS=0
FAILED=0
TOTAL=0

# Find all challenge directories
for dir in */; do
    # Skip if not a challenge directory (no Makefile)
    if [ ! -f "$dir/Makefile" ]; then
        continue
    fi

    TOTAL=$((TOTAL + 1))
    challenge_name=$(basename "$dir")

    echo ""
    echo -e "${YELLOW}Building $challenge_name...${NC}"

    # Enter directory and build
    cd "$dir"

    if make clean &> /dev/null && make &> /dev/null; then
        echo -e "${GREEN}✓ $challenge_name built successfully${NC}"
        SUCCESS=$((SUCCESS + 1))
    else
        echo -e "${RED}✗ $challenge_name failed to build${NC}"
        FAILED=$((FAILED + 1))
    fi

    cd ..
done

echo ""
echo "=============================="
echo "Build Summary:"
echo "  Total:   $TOTAL"
echo -e "  ${GREEN}Success: $SUCCESS${NC}"
echo -e "  ${RED}Failed:  $FAILED${NC}"

if [ $FAILED -eq 0 ]; then
    echo -e "${GREEN}All challenges built successfully!${NC}"
    exit 0
else
    echo -e "${RED}Some challenges failed to build${NC}"
    exit 1
fi
