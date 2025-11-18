# CTF Binary Exploitation Challenges

A comprehensive collection of 21 CTF-style binary exploitation challenges for testing automated exploit generation frameworks.

## Overview

These challenges are designed to test automated binary exploitation tools across various vulnerability classes. Each challenge includes:
- Vulnerable source code
- Makefile for compilation
- Dockerfile for containerization
- Detailed solution writeup

## Challenge Categories

### 1. Stack Buffer Overflows (Challenges 01-04)
- **01-stack-overflow-basic**: Classic buffer overflow with no protections
- **02-stack-overflow-canary-leak**: Stack canary with information leak
- **03-ret2libc**: Return-to-libc exploitation
- **04-stack-overflow-shellcode**: Shellcode injection on executable stack

### 2. Format String Vulnerabilities (Challenges 05-08)
- **05-format-string-leak**: Information leak via format string
- **06-format-string-write**: GOT overwrite with %n
- **07-format-string-stack-write**: Stack variable overwrite
- **08-format-string-blind**: Blind format string exploitation

### 3. ROP Challenges (Challenges 09-12)
- **09-rop-basic**: Basic ROP chain construction
- **10-ret2plt**: PLT/GOT leak and ret2libc
- **11-ret2syscall**: Direct syscall invocation
- **12-stack-pivot**: Stack pivoting to BSS

### 4. Integer/Index Vulnerabilities (Challenges 13-15)
- **13-integer-overflow**: Integer overflow bypass
- **14-negative-index**: Negative array index OOB write
- **15-signed-unsigned**: Signed/unsigned type confusion

### 5. Heap Exploitation (Challenges 16-18)
- **16-uaf-simple**: Use-after-free
- **17-double-free**: Double-free / tcache poisoning
- **18-heap-overflow**: Heap buffer overflow

### 6. Advanced/Combined (Challenges 19-21)
- **19-off-by-one**: Off-by-one null byte overflow
- **20-pwn-adventure**: Multiple vulnerabilities (game scenario)
- **21-race-condition**: TOCTOU race condition

## Quick Start

### Build All Challenges
```bash
cd ctf-challenges
./build_all.sh
```

### Build Individual Challenge
```bash
cd 01-stack-overflow-basic
make
```

### Run with Docker
```bash
cd 01-stack-overflow-basic
docker build -t challenge01 .
docker run -it challenge01
```

### Test All Challenges
```bash
./test_all.sh
```

## Vulnerability Detection Matrix

| Challenge | Type | Protections | Auto-Detectable | Difficulty |
|-----------|------|-------------|-----------------|------------|
| 01 | Stack Overflow | None | ✓ High | Easy |
| 02 | Stack Overflow + Leak | Canary | ✓ High | Easy-Med |
| 03 | Ret2libc | NX | ✓ High | Medium |
| 04 | Shellcode | None (NX disabled) | ✓ High | Easy-Med |
| 05 | Format String Leak | NX | ✓ High | Easy |
| 06 | Format String Write | NX, Partial RELRO | ✓ High | Medium |
| 07 | Format String Stack | NX | ✓ High | Easy-Med |
| 08 | Blind Format String | NX | ✓ Medium | Medium |
| 09 | ROP Basic | NX | ✓ Medium | Medium |
| 10 | Ret2PLT | NX, ASLR | ✓ Medium | Med-Hard |
| 11 | Ret2Syscall | NX (static) | ✓ Medium | Med-Hard |
| 12 | Stack Pivot | NX | ✓ Low | Med-Hard |
| 13 | Integer Overflow | NX | ✓ High | Easy-Med |
| 14 | Negative Index | NX | ✓ High | Easy |
| 15 | Signed/Unsigned | NX | ✓ High | Medium |
| 16 | Use-After-Free | NX | ✓ Medium | Easy-Med |
| 17 | Double-Free | NX | ✓ Medium | Medium |
| 18 | Heap Overflow | NX | ✓ Medium | Easy-Med |
| 19 | Off-by-One | NX | ✓ Medium | Medium |
| 20 | Multiple Vulns | NX | ✓ High | Medium |
| 21 | Race Condition | NX | ✓ Low | Med-Hard |

## Automated Tool Testing Guidelines

### Detection Phase
Your tool should identify:
1. **Vulnerability type** (overflow, format string, etc.)
2. **Input vector** (where to send payload)
3. **Win condition** (win function, shell, flag check)
4. **Protections** (NX, ASLR, canary, RELRO)

### Exploitation Phase
For each challenge, attempt to:
1. **Find crash** (fuzzing, static analysis)
2. **Determine exploitability** (RIP control, write primitive)
3. **Generate exploit** (offset calculation, ROP chain)
4. **Verify success** (flag retrieval, shell spawn)

### Success Metrics
- **Full Success**: Automated exploit retrieves flag
- **Partial Success**: Tool identifies vuln + provides guidance
- **Detection Only**: Tool finds crash but no exploitation
- **Failure**: Tool doesn't detect vulnerability

### Expected Automation Rates

Based on challenge difficulty and current tool capabilities:

| Difficulty | Expected Full Auto Rate |
|------------|-------------------------|
| Easy | 80-90% |
| Easy-Medium | 60-70% |
| Medium | 40-50% |
| Medium-Hard | 20-30% |
| Hard | <10% |

## Challenge Compilation

All challenges use consistent compilation flags:

```makefile
# Non-PIE, no canary (basic challenges)
gcc -m64 -fno-stack-protector -no-pie challenge.c -o challenge

# With canary
gcc -m64 -fstack-protector -no-pie challenge.c -o challenge

# Static binary (for ret2syscall)
gcc -m64 -fno-stack-protector -no-pie -static challenge.c -o challenge

# Format string (disable warnings)
gcc -m64 -fno-stack-protector -no-pie -Wno-format-security challenge.c -o challenge
```

## Testing Your Tool

### 1. Clone This Repository
```bash
git clone <repo-url>
cd ctf-challenges
```

### 2. Build All Challenges
```bash
chmod +x build_all.sh
./build_all.sh
```

### 3. Run Your Tool
```bash
# Example: Running hypothetical auto-exploit tool
for challenge in */challenge; do
    echo "Testing $challenge..."
    your-tool --binary $challenge --timeout 300
done
```

### 4. Collect Results
Document for each challenge:
- Was vulnerability detected? (Yes/No)
- Vulnerability type identified? (Correct/Incorrect/None)
- Exploit generated? (Yes/No)
- Exploit successful? (Yes/No)
- Time taken (seconds)
- Manual steps needed (if any)

## Recommended Tool Features

Based on these challenges, your automated tool should support:

### Essential Features
- [x] Crash detection (fuzzing)
- [x] RIP control detection
- [x] Offset calculation (pattern generation)
- [x] Win function detection
- [x] Protection detection (checksec)
- [x] Basic ROP chain generation

### Advanced Features
- [ ] Format string exploitation
- [ ] Libc leak and ret2libc
- [ ] Heap exploitation basics
- [ ] Information leak detection
- [ ] Multi-stage exploitation
- [ ] GOT overwrite

### Expert Features
- [ ] Stack pivoting
- [ ] Heap metadata corruption
- [ ] Race condition detection
- [ ] Constraint solving (symbolic execution)
- [ ] ASLR bypass strategies

## Directory Structure

```
ctf-challenges/
├── README.md                 # This file
├── build_all.sh              # Build all challenges
├── test_all.sh               # Test compilation
├── results_template.md       # Template for documenting results
├── 01-stack-overflow-basic/
│   ├── challenge.c
│   ├── Makefile
│   ├── Dockerfile
│   └── SOLUTION.md
├── 02-stack-overflow-canary-leak/
│   ├── ...
├── ...
└── 21-race-condition/
    └── ...
```

## Solution Writeups

Each challenge includes a `SOLUTION.md` with:
- Vulnerability explanation
- Protection analysis
- Exploitation strategy
- Complete exploit code
- Automated detection guidance
- Learning objectives

## Support Tools

### Recommended Tools for Testing
- **pwntools**: Python exploit development library
- **GDB + pwndbg/gef**: Debugging with heap/ROP helpers
- **ROPgadget/ropper**: ROP gadget finding
- **checksec**: Protection detection
- **AFL++**: Fuzzing
- **radare2/Ghidra**: Reverse engineering

### Detection Tools
- **AddressSanitizer (ASAN)**: Memory error detection
- **UndefinedBehaviorSanitizer (UBSan)**: Integer overflows
- **ThreadSanitizer (TSan)**: Race conditions
- **Valgrind**: Memory debugging

## Contributing

To add more challenges:
1. Follow the existing structure
2. Include all required files (source, Makefile, Dockerfile, solution)
3. Test with Docker
4. Document in README
5. Update detection matrix

## License

These challenges are created for educational purposes. Use them to:
- Test automated exploitation tools
- Learn binary exploitation
- Develop security testing frameworks
- Train CTF teams

## References

### Learning Resources
- "The Shellcoder's Handbook" - Chris Anley et al.
- "Hacking: The Art of Exploitation" - Jon Erickson
- "A Bug Hunter's Diary" - Tobias Klein
- Phrack Magazine (phrack.org)
- LiveOverflow YouTube channel
- pwn.college

### CTF Platforms
- pwnable.kr
- pwnable.tw
- ROP Emporium
- exploit.education
- picoCTF

### Tool Documentation
- pwntools: docs.pwntools.com
- ROPgadget: github.com/JonathanSalwan/ROPgadget
- AFL++: github.com/AFLplusplus/AFLplusplus

## Contact

For questions or issues with these challenges, please open an issue on the repository.

## Changelog

### Version 1.0 (Initial Release)
- 21 challenges across 6 categories
- Stack, format string, ROP, integer, heap, and advanced vulnerabilities
- Comprehensive documentation and solutions
- Docker support for all challenges
- Automated build scripts

---

**Note**: These challenges are intentionally vulnerable for educational purposes. Do not deploy them in production environments.
