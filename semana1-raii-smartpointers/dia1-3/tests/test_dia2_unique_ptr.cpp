#include "dia2/HandleDeleter.hpp"

#include <iostream>
#include <memory>
#include <utility>

int main() {
    // --- Ownership transfer ---
    auto source = std::make_unique<int[]>(2);
    source[0] = 7;
    int* original_address = source.get();

    std::unique_ptr<int[]> destination = std::move(source);

    if (source != nullptr) {
        std::cerr << "FAIL: after std::move, 'source' should be nullptr\n";
        return 1;
    }
    if (destination.get() != original_address || destination[0] != 7) {
        std::cerr << "FAIL: 'destination' should get the same pointer "
                     "and content that 'source' had\n";
        return 1;
    }

    // --- Custom deleter: runs when the scope ends ---
    int close_count = 0;
    {
        std::unique_ptr<RawHandle, HandleDeleter> handle(
            new RawHandle{1}, HandleDeleter(&close_count));
        if (close_count != 0) {
            std::cerr << "FAIL: the deleter shouldn't have run yet\n";
            return 1;
        }
    }
    if (close_count != 1) {
        std::cerr << "FAIL: the custom deleter should run exactly once "
                     "when the scope ends, it ran "
                  << close_count << " times\n";
        return 1;
    }

    // --- Custom deleter: also runs on reset() ---
    close_count = 0;
    std::unique_ptr<RawHandle, HandleDeleter> handle2(
        new RawHandle{2}, HandleDeleter(&close_count));
    handle2.reset();
    if (close_count != 1) {
        std::cerr << "FAIL: reset() should invoke the custom deleter\n";
        return 1;
    }

    std::cout << "OK: unique_ptr transfers ownership (the source is "
                 "left nullptr) and the custom deleter runs both in the "
                 "destructor and in reset()\n";
    return 0;
}
