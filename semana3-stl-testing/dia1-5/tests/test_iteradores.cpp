#include "Buffer.hpp"
#include "check.hpp"

#include <iterator>    // std::distance
#include <type_traits>
#include <utility>     // std::as_const

// Test: begin()/end() describe exactly the buffer's elements, and a
// const Buffer only hands out read-only iterators.
//
// Each function tests ONE behavior and follows arrange-act-assert:
// set up the data, run the operation, check the result.

void begin_to_end_spans_size_elements() {
    Buffer buffer(4);

    auto length = std::distance(buffer.begin(), buffer.end());

    CHECK(length == 4);
}

void empty_buffer_has_empty_range() {
    Buffer empty(0);

    CHECK(empty.empty());
    CHECK(empty.begin() == empty.end());
}

void range_for_writes_through_iterators() {
    Buffer buffer(3);

    for (int& value : buffer) {
        value = 7;
    }

    CHECK(buffer.at(0) == 7);
    CHECK(buffer.at(1) == 7);
    CHECK(buffer.at(2) == 7);
}

void const_buffer_gives_const_iterators() {
    // Checked at compile time: if someone removes the const overloads
    // of begin()/end(), this file stops compiling.
    Buffer buffer(1);
    static_assert(std::is_same_v<decltype(std::as_const(buffer).begin()),
                                 const int*>,
                  "a const Buffer must return const_iterator");
    static_assert(std::is_same_v<decltype(buffer.begin()), int*>,
                  "a mutable Buffer must return iterator");
}

void at_still_checks_bounds() {
    Buffer buffer(2);

    CHECK_THROWS(buffer.at(2), std::out_of_range);
    CHECK_THROWS(std::as_const(buffer).at(2), std::out_of_range);
}

int main() {
    begin_to_end_spans_size_elements();
    empty_buffer_has_empty_range();
    range_for_writes_through_iterators();
    const_buffer_gives_const_iterators();
    at_still_checks_bounds();
    return check::report("Buffer iterators");
}
