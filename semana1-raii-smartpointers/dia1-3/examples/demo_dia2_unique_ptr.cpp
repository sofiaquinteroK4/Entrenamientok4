#include "dia2/HandleDeleter.hpp"

#include <iostream>
#include <memory>
#include <utility>

// Day 2: unique_ptr — ownership transfer and custom deleters.

void demo_ownership_transfer() {
    std::cout << "\n--- Day 2: unique_ptr transfers ownership, it's not copied ---\n";

    auto data = std::make_unique<int[]>(3);
    data[0] = 10;
    std::cout << "  'data' points at " << data.get() << ", data[0] = "
              << data[0] << "\n";

    // unique_ptr has no copy constructor (it's deleted):
    //   auto copy = data; // <-- doesn't compile.
    // It can only be MOVED: the source is left as nullptr, the
    // destination gets the same pointer (no new allocation or data copy).
    std::unique_ptr<int[]> new_owner = std::move(data);

    std::cout << "  After std::move: 'data' = " << data.get()
              << " (nullptr), 'new_owner' = " << new_owner.get()
              << ", new_owner[0] = " << new_owner[0] << "\n";
}

void demo_custom_deleter() {
    std::cout << "\n--- Day 2: custom deleter ---\n";

    int close_count = 0;
    {
        // RawHandle simulates a resource from a C API that isn't
        // released with `delete`, but with its own "close" function.
        // HandleDeleter is the functor unique_ptr invokes instead.
        std::unique_ptr<RawHandle, HandleDeleter> handle(
            new RawHandle{42}, HandleDeleter(&close_count));
        std::cout << "  Handle opened with id=" << handle->id << "\n";
        // When this scope ends, HandleDeleter::operator() runs, just
        // like a RAII class's destructor.
    }
    std::cout << "  close_count = " << close_count
              << " (the custom deleter ran on its own when the scope ended)\n";
}

int main() {
    demo_ownership_transfer();
    demo_custom_deleter();
    std::cout << "\nProgram finished with no memory leaks.\n";
    return 0;
}
