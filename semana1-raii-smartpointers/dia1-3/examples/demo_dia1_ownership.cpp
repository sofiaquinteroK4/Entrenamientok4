#include "dia1/LifetimeTracker.hpp"

#include <iostream>

// Day 1: ownership review (stack vs heap) and why manual new/delete is
// risky.

void object_on_stack() {
    std::cout << "  Inside the scope, before construction: alive_count() = "
              << LifetimeTracker::alive_count() << "\n";
    LifetimeTracker t; // lives on the stack
    (void)t;
    std::cout << "  Inside the scope, with 't' alive: alive_count() = "
              << LifetimeTracker::alive_count() << "\n";
    // When this function returns, 't' goes out of scope and its
    // destructor runs automatically. Nobody has to remember anything.
}

LifetimeTracker* object_on_heap() {
    // Risk: if this function returns the pointer and the caller
    // "forgets" the delete, this object is orphaned forever: there's
    // no way to reach it or free it anymore -> memory leak. Unlike the
    // stack, the heap NEVER frees itself.
    return new LifetimeTracker();
}

void demo_stack_vs_heap() {
    std::cout << "\n--- Day 1: stack vs heap ---\n";

    std::cout << "object_on_stack():\n";
    object_on_stack();
    std::cout << "After the function returned: alive_count() = "
              << LifetimeTracker::alive_count()
              << " (the destructor already ran on its own, without "
                 "anyone asking)\n";

    std::cout << "\nobject_on_heap():\n";
    LifetimeTracker* p = object_on_heap();
    std::cout << "After the function returned: alive_count() = "
              << LifetimeTracker::alive_count()
              << " (it's still alive! the heap doesn't free itself, an "
                 "explicit 'delete' is needed)\n";

    delete p; // manual release: fragile and easy to forget in real code
    std::cout << "After 'delete p;': alive_count() = "
              << LifetimeTracker::alive_count() << "\n";
    std::cout << "That's why, instead of remembering a 'delete' for "
                 "every 'new', the following weeks wrap the heap in a "
                 "RAII class (RaiiBuffer, Buffer) or in a smart pointer "
                 "(unique_ptr, shared_ptr): the release becomes "
                 "automatic, just like on the stack.\n";
}

int main() {
    demo_stack_vs_heap();
    std::cout << "\nRepo set up with CMake + CTest + .gitignore for the "
                 "rest of the month (see the repo root README).\n";
    return 0;
}
