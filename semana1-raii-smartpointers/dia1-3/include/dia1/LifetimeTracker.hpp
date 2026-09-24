#pragma once

// Day 1: ownership review (stack vs heap).
//
// LifetimeTracker doesn't manage any resource of its own; it just
// keeps a static count of how many instances are currently alive. It
// exists to OBSERVE, with real data, when an object is constructed and
// when it's destroyed depending on where it lives (stack vs heap).

class LifetimeTracker {
public:
    LifetimeTracker() { ++alive_count_; }
    LifetimeTracker(const LifetimeTracker&) { ++alive_count_; }
    LifetimeTracker(LifetimeTracker&&) noexcept { ++alive_count_; }
    LifetimeTracker& operator=(const LifetimeTracker&) = default;
    LifetimeTracker& operator=(LifetimeTracker&&) noexcept = default;
    ~LifetimeTracker() { --alive_count_; }

    static int alive_count() { return alive_count_; }

private:
    static inline int alive_count_ = 0;
};
