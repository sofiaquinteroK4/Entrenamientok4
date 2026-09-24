#include "RaiiBuffer.hpp"

#include <cstdlib>
#include <iostream>

// Test: construction, size, valid access, and out-of-range access.
// No external framework: each failure prints the reason and returns 1,
// which makes `ctest` mark the case as FAILED.
int main() {
    RaiiBuffer buffer(5);

    if (buffer.size() != 5) {
        std::cerr << "FAIL: expected size() 5, got " << buffer.size() << "\n";
        return 1;
    }

    buffer.at(0) = 42;
    if (buffer.at(0) != 42) {
        std::cerr << "FAIL: expected at(0) 42, got " << buffer.at(0) << "\n";
        return 1;
    }

    bool threw = false;
    try {
        buffer.at(10) = 1;
    } catch (const std::out_of_range&) {
        threw = true;
    }
    if (!threw) {
        std::cerr << "FAIL: at(10) should have thrown std::out_of_range\n";
        return 1;
    }

    std::cout << "OK: construction, size(), valid at(), and out-of-range at()\n";
    return 0;
}
