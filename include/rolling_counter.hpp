#pragma once
#include <cstdint>
#include <deque>
// HW8 — sliding-window event counter. count() is called with a non-decreasing clock.
//
// Edge case that costs most people a test (see labs/week08.md step 5): ts_ns and
// now_ns are UNSIGNED. Early on, now_ns <= window_ns and the mathematical cutoff
// (now - window) is negative — there is no uint64_t that means that, and there is
// no safe value to clamp it to. Only compute the subtraction when it is
// meaningful; do not expire anything before then.
// struct RollingCounter {
//     explicit RollingCounter(uint64_t window_ns) { (void)window_ns;
//         // TODO(student): ring/deque of timestamps; amortized O(1).
//     }
//     void add(uint64_t ts_ns) { (void)ts_ns; /* TODO(student) */ }
//     uint64_t count(uint64_t now_ns) { (void)now_ns; return 0; /* TODO: events with ts > now-window */ }
// };

struct RollingCounter {
    explicit RollingCounter(uint64_t window_ns) : window_(window_ns) {}
 
    // Timestamps are appended in arrival order (non-decreasing), so the deque
    // stays sorted and the oldest event is always at the front.
    void add(uint64_t ts_ns) { events_.push_back(ts_ns); }
 
    // Counts events with ts > now - window. Equivalently, an event expires
    // when ts <= now - window.
    uint64_t count(uint64_t now_ns) {
        // Guard the unsigned subtraction: only compute now - window when
        // now >= window (result is then a valid uint64_t, possibly 0).
        // If now < window the true cutoff is negative, so nothing can expire.
        if (now_ns >= window_) {
            const uint64_t cutoff = now_ns - window_;
            // Each event is popped at most once over its lifetime => amortized O(1).
            while (!events_.empty() && events_.front() <= cutoff) {
                events_.pop_front();
            }
        }
        return events_.size();
    }
 
private:
    uint64_t window_;
    std::deque<uint64_t> events_;
};
