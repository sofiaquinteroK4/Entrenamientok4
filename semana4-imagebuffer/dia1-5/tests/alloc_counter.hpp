#pragma once

// Counts live heap arrays, so a test can assert that every new[] had
// its matching delete[].
//
// Why: `leaks` (macOS) and valgrind scan memory conservatively. A stale
// copy of a pointer left on the stack makes a leaked block look
// "still reachable", so a move assignment that overwrites data_ without
// freeing the old block can slip past them. Counting allocations is
// exact and works the same on every platform and build.
//
// It works by replacing the global array operator new[]/delete[], which
// is what ImageBuffer uses. The replacements are real (non-inline)
// definitions, so include this header from exactly ONE .cpp file per
// test executable.

#include <cstddef>
#include <cstdlib>
#include <new>

namespace alloc_counter {
inline std::size_t live_arrays = 0;
} // namespace alloc_counter

void* operator new[](std::size_t size) {
    void* p = std::malloc(size > 0 ? size : 1);
    if (p == nullptr) {
        throw std::bad_alloc();
    }
    ++alloc_counter::live_arrays;
    return p;
}

void operator delete[](void* p) noexcept {
    if (p != nullptr) {
        --alloc_counter::live_arrays;
        std::free(p);
    }
}

void operator delete[](void* p, std::size_t) noexcept {
    operator delete[](p);
}
