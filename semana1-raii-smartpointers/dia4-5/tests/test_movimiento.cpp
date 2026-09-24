#include "RaiiBuffer.hpp"

#include <iostream>
#include <utility>

// Test: after moving (construction and assignment), the source is left
// empty (size() == 0, any at() throws) and the destination keeps the
// value.
int main() {
    // --- Move constructor ---
    RaiiBuffer source(3);
    source.at(0) = 100;

    RaiiBuffer destination(std::move(source));

    if (destination.size() != 3 || destination.at(0) != 100) {
        std::cerr << "FAIL: move constructor did not transfer the value\n";
        return 1;
    }
    if (source.size() != 0) {
        std::cerr << "FAIL: move constructor did not leave 'source' empty "
                     "(size() = " << source.size() << ")\n";
        return 1;
    }
    bool source_throws = false;
    try {
        source.at(0);
    } catch (const std::out_of_range&) {
        source_throws = true;
    }
    if (!source_throws) {
        std::cerr << "FAIL: at() on a moved-from 'source' should have thrown\n";
        return 1;
    }

    // --- Move assignment ---
    RaiiBuffer other(2);
    other.at(0) = 7;
    RaiiBuffer receiver(1);
    receiver = std::move(other);

    if (receiver.size() != 2 || receiver.at(0) != 7) {
        std::cerr << "FAIL: move assignment did not transfer the value\n";
        return 1;
    }
    if (other.size() != 0) {
        std::cerr << "FAIL: move assignment did not leave 'other' empty\n";
        return 1;
    }

    std::cout << "OK: move constructor and move assignment leave the "
                 "source empty and transfer the value to the destination\n";
    return 0;
}
