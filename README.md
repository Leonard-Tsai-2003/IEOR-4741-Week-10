# IEOR 4741 - Week 10: SPSC Lock-Free Ring Buffer

A high-performance, single-producer single-consumer (SPSC) lock-free ring buffer implemented in modern C++. This data structure is optimized for low-latency concurrent processing where one thread writes data and another reads it without the overhead of mutexes or context switches.

## Features & Implementation Details

Located in `include/spsc_ring.hpp`, the lock-free SPSC queue incorporates critical hardware-aware optimizations:

- **Lock-Free Concurrency**: Uses `std::atomic` for `head` and `tail` pointers with fine-grained `std::memory_order_acquire` and `std::memory_order_release` semantics to ensure correct visibility and synchronization across threads without locking.
- **Cache-Line Padding (False Sharing Prevention)**: Atomic indices (`head` and `tail`) are aligned and padded to 64 bytes (the standard cache-line boundary) to prevent *false sharing*, ensuring the producer core and consumer core do not invalidate each other's L1/L2 cache lines.
- **Power-of-Two Masking**: Array capacity is restricted to powers of two ($N = 2^k$). Fast bitwise AND operations (`index & (capacity - 1)`) replace the costly integer modulo operation ($index \pmod N$).
- **Stress-Tested Reliability**: Designed to handle millions of items under concurrent multi-threaded execution while guaranteeing non-blocking operation, FIFO ordering, and zero data loss.

---

## Directory Structure

```text
.
├── include/
│   └── spsc_ring.hpp    # Header implementation of the SPSC lock-free ring buffer
├── Makefile             # Build automation script
└── README.md            # Project documentation
```

---

## Prerequisites

- **C++ Compiler**: `g++` or `clang++` with C++17 support.
- **Build System**: GNU `make`.

---

## Building and Running Tests

To build and execute the verification and stress-testing suite for the SPSC ring buffer, run:

```bash
make spsc
```

### Expected Output

Upon running `make spsc`, the test suite verifies edge cases, wrap-around semantics, full/empty conditions, and FIFO ordering over large volume operations:

```text
RESULT|basic|pass|push/pop/empty behave
RESULT|empty_pop|pass|pop on empty returns false
RESULT|full|pass|filled 64 then push=false, full()=true
RESULT|fifo|pass|FIFO order preserved over 1000 values
RESULT|wrap|pass|wrap-around correct over 200k cycles
```

---

## Technical Highlights

### 1. Acquire-Release Semantics
- **Producer (`push`)**: Updates `head` using `std::memory_order_release` after writing the item. Reads `tail` using `std::memory_order_acquire`.
- **Consumer (`pop`)**: Updates `tail` using `std::memory_order_release` after consuming the item. Reads `head` using `std::memory_order_acquire`.

### 2. Preventing False Sharing
```cpp
// Cache-aligned to avoid hardware destructive interference
alignas(64) std::atomic<size_t> head_{0};
alignas(64) std::atomic<size_t> tail_{0};
```
By isolating `head_` and `tail_` into separate 64-byte cache lines, thread cache contention is virtually eliminated.