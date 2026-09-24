#pragma once

#include <memory>

// Day 3: shared_ptr (reference counting) and the circular reference
// problem.
//
// CyclicNodeBroken: if two nodes point at each other with shared_ptr,
// each one's reference count never reaches 0, even after the code
// that created them stops using them -> the destructor never runs
// -> memory leak.
struct CyclicNodeBroken {
    static inline int alive_count = 0;
    std::shared_ptr<CyclicNodeBroken> next;

    CyclicNodeBroken() { ++alive_count; }
    ~CyclicNodeBroken() { --alive_count; }
};

// CyclicNodeFixed: one of the two sides uses weak_ptr, a reference
// that "observes" the object without adding to the shared_ptr count.
// With no shared_ptr cycle, both nodes can reach 0 references and
// destroy themselves.
struct CyclicNodeFixed {
    static inline int alive_count = 0;
    std::shared_ptr<CyclicNodeFixed> next; // owns "forward"
    std::weak_ptr<CyclicNodeFixed> prev;   // observes "backward", doesn't own

    CyclicNodeFixed() { ++alive_count; }
    ~CyclicNodeFixed() { --alive_count; }
};
