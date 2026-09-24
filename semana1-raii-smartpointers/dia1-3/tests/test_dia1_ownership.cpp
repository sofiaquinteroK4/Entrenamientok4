#include "dia1/LifetimeTracker.hpp"

#include <iostream>

// Test: a stack object destroys itself when its scope ends; a heap
// object survives until it's explicitly freed.

void construct_on_stack() {
    LifetimeTracker t;
    (void)t;
    // when this function returns, 't' is destroyed automatically
}

LifetimeTracker* construct_on_heap() {
    return new LifetimeTracker();
}

int main() {
    if (LifetimeTracker::alive_count() != 0) {
        std::cerr << "FAIL: initial alive_count() should be 0\n";
        return 1;
    }

    construct_on_stack();
    if (LifetimeTracker::alive_count() != 0) {
        std::cerr << "FAIL: a stack object should destroy itself when "
                     "its scope ends\n";
        return 1;
    }

    LifetimeTracker* heap_obj = construct_on_heap();
    if (LifetimeTracker::alive_count() != 1) {
        std::cerr << "FAIL: a heap object should still be alive after "
                     "the function that created it returned\n";
        return 1;
    }

    delete heap_obj; // without this manual delete, the object above would leak
    if (LifetimeTracker::alive_count() != 0) {
        std::cerr << "FAIL: a manual delete should free the heap object\n";
        return 1;
    }

    std::cout << "OK: a stack object destroys itself when its scope "
                 "ends; a heap object survives until it's freed by hand\n";
    return 0;
}
