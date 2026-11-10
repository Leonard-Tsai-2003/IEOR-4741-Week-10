#pragma once

#include <atomic>
#include <cstddef>
#include <cstdint>

// HW10 / Project Phase 3 — single-producer/single-consumer lock-free ring buffer.
struct SPSCRing {
    static constexpr std::size_t kCacheLineSize = 64;

    const std::size_t capacity_;
    const std::size_t mask_;
    std::uint64_t* const buffer_;

    // Head: Producer write index on its own cache line
    alignas(kCacheLineSize) std::atomic<std::size_t> head_{0};

    // Tail: Consumer read index on its own cache line
    alignas(kCacheLineSize) std::atomic<std::size_t> tail_{0};

    // Trailing padding to prevent false sharing with adjacent allocations
    alignas(kCacheLineSize) char padding_[kCacheLineSize];

private:
    static std::size_t round_up_pow2(std::size_t v) {
        if (v == 0) return 1;
        v--;
        v |= v >> 1;
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;
        if (sizeof(std::size_t) > 4) {
            v |= v >> 32;
        }
        return v + 1;
    }

public:
    explicit SPSCRing(std::size_t capacity_pow2)
        : capacity_(round_up_pow2(capacity_pow2)),
          mask_(capacity_ - 1),
          buffer_(new std::uint64_t[capacity_]) {}

    ~SPSCRing() {
        delete[] buffer_;
    }

    SPSCRing(const SPSCRing&) = delete;
    SPSCRing& operator=(const SPSCRing&) = delete;

    // Producer thread updates head_ (write index)
    bool push(std::uint64_t v) {
        const std::size_t h = head_.load(std::memory_order_relaxed);
        const std::size_t t = tail_.load(std::memory_order_acquire);

        if (h - t == capacity_) {
            return false; // Full
        }

        buffer_[h & mask_] = v;
        head_.store(h + 1, std::memory_order_release);
        return true;
    }

    // Consumer thread updates tail_ (read index)
    bool pop(std::uint64_t& out) {
        const std::size_t t = tail_.load(std::memory_order_relaxed);
        const std::size_t h = head_.load(std::memory_order_acquire);

        if (h == t) {
            return false; // Empty
        }

        out = buffer_[t & mask_];
        tail_.store(t + 1, std::memory_order_release);
        return true;
    }

    bool empty() const {
        return head_.load(std::memory_order_relaxed) == tail_.load(std::memory_order_relaxed);
    }

    bool full() const {
        return (head_.load(std::memory_order_relaxed) - tail_.load(std::memory_order_relaxed)) == capacity_;
    }
};