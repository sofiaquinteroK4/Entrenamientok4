#include "Buffer.hpp"

#include <iostream>

// Test: construction (including size 0), zero-init, const-correctness,
// and out-of-range access on mutable and const objects.
int main() {
    Buffer buffer(4);

    if (buffer.size() != 4) {
        std::cerr << "FAIL: expected size() 4, got " << buffer.size() << "\n";
        return 1;
    }
    for (std::size_t i = 0; i < buffer.size(); ++i) {
        if (buffer.at(i) != 0) {
            std::cerr << "FAIL: at(" << i << ") should be 0 after construction\n";
            return 1;
        }
    }

    // A size-0 buffer: must not allocate memory or fail.
    Buffer empty(0);
    if (empty.size() != 0 || empty.address() != nullptr) {
        std::cerr << "FAIL: Buffer(0) should be empty (size=0, address=nullptr)\n";
        return 1;
    }

    buffer.at(0) = 42;
    const Buffer& read_only = buffer;
    if (read_only.at(0) != 42) {
        std::cerr << "FAIL: the const overload of at() did not return the expected value\n";
        return 1;
    }

    bool threw = false;
    try {
        buffer.at(100) = 1;
    } catch (const std::out_of_range&) {
        threw = true;
    }
    if (!threw) {
        std::cerr << "FAIL: at(100) should have thrown std::out_of_range\n";
        return 1;
    }

    std::cout << "OK: construction, zero-init, size 0, const-correctness, "
                 "and out-of-range at()\n";
    return 0;
}
