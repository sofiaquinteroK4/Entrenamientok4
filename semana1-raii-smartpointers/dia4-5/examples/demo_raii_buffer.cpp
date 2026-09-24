#include "RaiiBuffer.hpp"

#include <iostream>

// ---------------------------------------------------------
// Demo program: manually validated with prints that show
// everything is allocated and freed correctly, in the expected
// order. Automated tests with assertions live in tests/.
// ---------------------------------------------------------
void basic_example() {
    std::cout << "\n--- basic_example() ---\n";
    RaiiBuffer buffer(5);
    buffer.at(0) = 42;
    std::cout << "buffer.at(0) = " << buffer.at(0) << "\n";
    // When this function returns, the destructor frees the buffer on its own.
}

void move_example() {
    std::cout << "\n--- move_example() ---\n";
    RaiiBuffer original(3);
    original.at(0) = 100;

    std::cout << "Moving 'original' into 'moved'...\n";
    RaiiBuffer moved(std::move(original));
    // 'original' is now empty (data_ == nullptr), 'moved' is the real owner.

    std::cout << "moved.at(0) = " << moved.at(0) << "\n";
    // At the end of the function: 'original's destructor (does nothing)
    // and 'moved's destructor (frees the real memory).
}

void exception_example() {
    std::cout << "\n--- exception_example() ---\n";
    try {
        RaiiBuffer buffer(2);
        buffer.at(10) = 1; // this throws std::out_of_range
    } catch (const std::out_of_range& e) {
        std::cout << "Exception caught: " << e.what() << "\n";
        std::cout << "(Even so, 'buffer's destructor already ran "
                     "before we got here -> no leaks)\n";
    }
}

int main() {
    basic_example();
    move_example();
    exception_example();
    std::cout << "\nProgram finished with no memory leaks.\n";
    return 0;
}
