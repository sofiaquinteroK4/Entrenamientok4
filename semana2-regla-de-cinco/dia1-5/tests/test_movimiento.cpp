#include "Buffer.hpp"

#include <iostream>
#include <utility>

// Test: move constructor and move assignment transfer the pointer
// (same address, no new allocation) and leave the source empty.
int main() {
    // --- Move constructor ---
    Buffer source(3);
    source.at(0) = 222;
    const void* original_address = source.address();

    Buffer destination(std::move(source));

    if (destination.address() != original_address) {
        std::cerr << "FAIL: move constructor should reuse the same address\n";
        return 1;
    }
    if (destination.size() != 3 || destination.at(0) != 222) {
        std::cerr << "FAIL: move constructor did not preserve size/content\n";
        return 1;
    }
    if (source.address() != nullptr || source.size() != 0) {
        std::cerr << "FAIL: move constructor did not leave 'source' empty\n";
        return 1;
    }

    // --- Move assignment ---
    Buffer other(2);
    other.at(0) = 333;
    const void* other_address = other.address();
    Buffer receiver(5);

    receiver = std::move(other);

    if (receiver.address() != other_address) {
        std::cerr << "FAIL: move assignment should reuse the same address\n";
        return 1;
    }
    if (receiver.size() != 2 || receiver.at(0) != 333) {
        std::cerr << "FAIL: move assignment did not preserve size/content\n";
        return 1;
    }
    if (other.address() != nullptr || other.size() != 0) {
        std::cerr << "FAIL: move assignment did not leave 'other' empty\n";
        return 1;
    }

    std::cout << "OK: move constructor and move assignment transfer "
                 "without duplicating and leave the source empty\n";
    return 0;
}
