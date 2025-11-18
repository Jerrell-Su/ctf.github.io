# Challenge 19: Off-by-One

## Difficulty: Medium

## Vulnerability Type: Off-by-One Null Byte Overflow

## Description
Classic off-by-one vulnerability where a null byte is written one position past the buffer, allowing partial overwrite of saved RBP.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial

## Vulnerability Analysis

### The Bug
```c
char buffer[64];
read(0, buffer, size);     // Read 'size' bytes
buffer[size] = '\0';       // Write null at position 'size'
```

If `size = 64`:
- `read()` fills buffer[0..63] (64 bytes)
- `buffer[64] = '\0'` writes null byte at buffer[64]
- buffer[64] is the **first byte of saved RBP**!

### Stack Layout
```
[buffer: 64 bytes][saved RBP: 8 bytes][return address: 8 bytes]
                  ^
                  Off-by-one null byte written here!
```

### Why This Matters
Writing a null byte to the first byte of saved RBP partially corrupts it:
```
Original RBP: 0x00007fffffffdxxx
After null:   0x00007fffffffd000  (lower byte zeroed)
```

This changes the stack frame base pointer!

## Exploitation Strategy

### Traditional Off-by-One Exploitation
The null byte overflow can:
1. Partially overwrite saved RBP
2. When function returns, RBP is restored with modified value
3. If caller function uses RBP-relative addressing, we control data

### For This Challenge
We can:
1. Fill buffer completely (64 bytes)
2. Trigger off-by-one to corrupt saved RBP
3. Use stack layout knowledge to craft payload

### Simplified Approach
Since we can control 64 bytes + 1 null byte, and this is a simple challenge:
```python
# Actually, we can just overflow normally
# by using size=64, we get to write to buffer[64]
# But we need more control...

# Better: Use the fact that we can partially control RBP
# Then in some cases, when function returns and RBP is used,
# we can influence the data being accessed
```

### Alternative: Information Leak + Second Stage
Some off-by-one exploits require:
1. Leak stack address
2. Calculate precise overwrite
3. Use second vulnerability

### Practical Exploitation
For CTF, often we combine off-by-one with other techniques:
- Stack pivoting
- Partial RIP overwrite (if lucky with address alignment)
- Chaining multiple calls

### Complete Exploit (Conceptual)
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9019)
else:
    p = process('./challenge')

win_addr = elf.symbols['win']

# Method 1: Try to exploit off-by-one
# Send size = 64
p.sendlineafter(b'(0-64): ', b'64')

# Fill buffer, trying to control saved RBP behavior
# In this simple case, we might need to:
# 1. Fill with pattern
# 2. Hope for lucky alignment
# 3. Or combine with other bugs

# For educational CTF, often the challenge would have
# a second call to vuln() or other way to leverage
# the corrupted RBP

# Simplified: If challenge allows multiple inputs,
# corrupt RBP first, then overflow on second call

# Build payload
payload = b'A' * 64

p.sendafter(b'data: ', payload)

p.interactive()
```

### Better Scenario (if vuln() called twice)
```python
# Call 1: Corrupt RBP with off-by-one
p.sendlineafter(b'size', b'64')
p.sendafter(b'data', b'A' * 64)

# Call 2: Now RBP is corrupted, exploit it
# (Would need specific program structure)
```

## Automated Detection

### Static Analysis
```python
# Pattern to detect:
buffer[index] = value
# Where index can equal buffer size
```

### Dynamic Analysis (ASAN)
```bash
gcc -fsanitize=address challenge.c
./challenge
# Input size=64, data=64 A's
# ASAN will report: stack-buffer-overflow (1 byte)
```

### Fuzzing
AFL++ with ASAN will find this quickly:
```bash
afl-clang-fast -fsanitize=address challenge.c
afl-fuzz -m none -i in -o out ./challenge
```

## Off-by-One Variants

### Null Byte Overflow (This Challenge)
```c
buffer[size] = '\0';  // Off-by-one null
```

### Arithmetic Off-by-One
```c
for (int i = 0; i <= MAX; i++)  // Should be i < MAX
    buffer[i] = data[i];
```

### Heap Off-by-One
```c
chunk = malloc(SIZE);
memset(chunk, 0, SIZE + 1);  // Overwrites heap metadata!
```

## Real-World Examples
- CVE-2021-3156 (Sudo): Heap-based off-by-one
- CVE-2017-7529 (Nginx): Off-by-one in range filter
- CVE-2015-8317 (libxml2): Off-by-one buffer overflow
- Many glibc bugs

## Prevention

### Careful Indexing
```c
// Wrong
buffer[size] = '\0';

// Right
if (size < sizeof(buffer)) {
    buffer[size] = '\0';
}

// Better
buffer[sizeof(buffer) - 1] = '\0';
```

### Use Safe Functions
```c
// Instead of manual null termination
strncpy(buffer, source, sizeof(buffer));
buffer[sizeof(buffer) - 1] = '\0';  // Ensure termination
```

### Compiler Warnings
```bash
gcc -Warray-bounds -Wstringop-overflow
```

## Learning Objectives
- Off-by-one errors
- Subtle boundary conditions
- Null byte significance
- Stack frame pointer corruption
- Importance of precise bounds checking

## Debugging Off-by-One
```gdb
# Set breakpoint before and after write
b *vuln+XX
run

# Examine buffer and beyond
x/20gx $rbp-64  # Show buffer + saved RBP + return

# Step through to see corruption
si
x/gx $rbp  # See modified RBP value
```

## Advanced Exploitation

### Partial Overwrite
If addresses have null bytes, off-by-one can:
- Zero out address bytes
- Redirect to nearby code
- Create unintended behavior

### Heap Off-by-One
Much more powerful in heap context:
```
[chunk A][chunk B size field]
         ^
         Off-by-one can modify size!
```

Can lead to:
- Chunk overlapping
- Consolidation attacks
- Arbitrary read/write

## Further Reading
- "The Poisoned NUL Byte" (Phrack)
- "Once upon a free()" (Phrack 57)
- "Off-by-one vulnerabilities" - OWASP
