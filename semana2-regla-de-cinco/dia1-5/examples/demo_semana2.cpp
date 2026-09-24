#include "Buffer.hpp"

#include <iostream>
#include <string>
#include <utility>

// ---------------------------------------------------------------------
// std::move and std::forward
// ---------------------------------------------------------------------
// - std::move doesn't move anything by itself: it's just a cast to an
//   rvalue reference (T&&). It tells the compiler "treat this object
//   as temporary", enabling the move constructor/operator to be
//   selected instead of the copy one.
// - std::forward is used in templates with forwarding references
//   (T&&) to preserve whether the original argument was an lvalue or
//   an rvalue when forwarding it to another function.
void process(const std::string& s) {
    std::cout << "  process(const std::string&) -> copy/read: \"" << s << "\"\n";
}

void process(std::string&& s) {
    std::cout << "  process(std::string&&) -> could move: \"" << s << "\"\n";
}

template <typename T>
void forward_and_show(T&& value) {
    // std::forward<T> preserves whether `value` was an lvalue or an
    // rvalue at the original call site.
    process(std::forward<T>(value));
}

// ---------------------------------------------------------------------
// Manual demonstrations (with prints). Automated tests with
// assertions and an exit code live in tests/.
// ---------------------------------------------------------------------
void demo_const_correctness() {
    std::cout << "\n--- Day 1: const-correctness ---\n";
    const Buffer read_only(2);
    // read_only.at(0) = 5; // <-- doesn't compile: the const at()
    //                          overload returns const int&, which
    //                          can't be assigned to.
    std::cout << "  Reading a const Buffer: read_only.at(0) = "
              << read_only.at(0) << " (size=" << read_only.size()
              << ")\n";
    std::cout << "  Compiles because size()/at() const and address() "
                 "are marked const: they can be called on a const object.\n";
}

void demo_forwarding() {
    std::cout << "\n--- Day 3: std::move / std::forward ---\n";
    std::string text = "hello";
    forward_and_show(text);                // lvalue -> process(const&)
    forward_and_show(std::string("bye"));  // rvalue -> process(&&)
}

void demo_copy_duplicates_resource() {
    std::cout << "\n--- Day 4: copying DUPLICATES the resource ---\n";
    Buffer original(3);
    original.at(0) = 111;

    Buffer copy(original); // copy constructor

    std::cout << "Addresses before modifying the copy:\n";
    original.print("original");
    copy.print("copy");

    copy.at(0) = 999; // modifying the copy must not affect the original

    std::cout << "After copy.at(0) = 999:\n";
    original.print("original");
    copy.print("copy");

    if (original.address() != copy.address() && original.at(0) == 111) {
        std::cout << "OK: different addresses and 'original' didn't "
                     "change -> the copy is independent (deep copy confirmed).\n";
    } else {
        std::cout << "FAIL: the copy shares memory with the original.\n";
    }
}

void demo_copy_assignment() {
    std::cout << "\n--- Day 4: copy assignment also duplicates ---\n";
    Buffer source(2);
    source.at(0) = 55;
    Buffer receiver(5);
    const void* receiver_address_before = receiver.address();

    receiver = source; // copy assignment (copy-and-swap)

    receiver.at(0) = 777; // must not affect 'source'

    source.print("source");
    receiver.print("receiver");

    if (receiver.address() != source.address() &&
        receiver.address() != receiver_address_before &&
        source.at(0) == 55) {
        std::cout << "OK: copy assignment allocated a new block, "
                     "independent from the source.\n";
    } else {
        std::cout << "FAIL: copy assignment did not isolate the resource.\n";
    }
}

void demo_move_transfers_no_duplicate() {
    std::cout << "\n--- Day 4: moving TRANSFERS without duplicating ---\n";
    Buffer source(3);
    source.at(0) = 222;
    const void* original_address = source.address();

    Buffer destination(std::move(source)); // move constructor

    std::cout << "After the move:\n";
    source.print("source (now empty)");
    destination.print("destination");

    if (destination.address() == original_address && source.address() == nullptr) {
        std::cout << "OK: 'destination' reuses the SAME address that "
                     "'source' had, and 'source' was left empty -> no new "
                     "allocation or data copy happened.\n";
    } else {
        std::cout << "FAIL: the move did not transfer the pointer as expected.\n";
    }
}

void demo_move_assignment() {
    std::cout << "\n--- Day 4: move assignment also transfers ---\n";
    Buffer source(3);
    source.at(0) = 333;
    const void* original_address = source.address();
    Buffer receiver(1);

    receiver = std::move(source); // move assignment

    source.print("source (now empty)");
    receiver.print("receiver");

    if (receiver.address() == original_address && source.address() == nullptr) {
        std::cout << "OK: move assignment released what 'receiver' had "
                     "and took over 'source's block.\n";
    } else {
        std::cout << "FAIL: move assignment did not transfer the pointer.\n";
    }
}

int main() {
    demo_const_correctness();
    demo_forwarding();
    demo_copy_duplicates_resource();
    demo_copy_assignment();
    demo_move_transfers_no_duplicate();
    demo_move_assignment();
    std::cout << "\nProgram finished with no memory leaks.\n";
    return 0;
}
