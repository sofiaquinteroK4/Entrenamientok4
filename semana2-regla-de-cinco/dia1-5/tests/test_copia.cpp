#include "Buffer.hpp"

#include <iostream>

// Test: copy constructor and copy assignment perform a DEEP copy ->
// distinct addresses and full independence after modifying the copy.
int main() {
    // --- Copy constructor ---
    Buffer original(3);
    original.at(0) = 111;

    Buffer copy(original);

    if (copy.address() == original.address()) {
        std::cerr << "FAIL: copy constructor shares memory with the original\n";
        return 1;
    }
    if (copy.size() != original.size() || copy.at(0) != 111) {
        std::cerr << "FAIL: copy constructor did not preserve size/content\n";
        return 1;
    }

    copy.at(0) = 999;
    if (original.at(0) != 111) {
        std::cerr << "FAIL: modifying the copy affected the original "
                     "(not an independent copy)\n";
        return 1;
    }

    // --- Copy assignment (copy-and-swap) ---
    Buffer receiver(1);
    const void* receiver_address_before = receiver.address();
    receiver = original;

    if (receiver.address() == original.address()) {
        std::cerr << "FAIL: copy assignment shares memory with the source\n";
        return 1;
    }
    if (receiver.address() == receiver_address_before) {
        std::cerr << "FAIL: copy assignment did not free the receiver's previous block\n";
        return 1;
    }
    if (receiver.size() != original.size() || receiver.at(0) != 111) {
        std::cerr << "FAIL: copy assignment did not preserve size/content\n";
        return 1;
    }

    receiver.at(0) = 42;
    if (original.at(0) != 111) {
        std::cerr << "FAIL: modifying the receiver affected the original\n";
        return 1;
    }

    // --- Self-assignment ---
    // Self-assignment is intentional: copy-and-swap must handle it
    // without corrupting state, with no `if (this == &other)` check.
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
    receiver = receiver;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
    if (receiver.at(0) != 42 || receiver.size() != 3) {
        std::cerr << "FAIL: self-assignment corrupted the state\n";
        return 1;
    }

    std::cout << "OK: copy constructor and copy assignment perform an "
                 "independent deep copy; self-assignment is safe\n";
    return 0;
}
