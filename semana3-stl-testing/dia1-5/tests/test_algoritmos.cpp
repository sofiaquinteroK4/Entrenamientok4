#include "Buffer.hpp"
#include "check.hpp"

#include <algorithm>
#include <functional> // std::greater
#include <numeric>    // std::iota, std::accumulate

// Test: standard algorithms work on a Buffer and give the expected
// results, including the edge cases (empty buffer, "not found").

// Builds a Buffer with the given values. Used as the "arrange" step.
template <std::size_t N>
Buffer make_buffer(const int (&values)[N]) {
    Buffer buffer(N);
    std::copy(values, values + N, buffer.begin());
    return buffer;
}

void fill_sets_every_element() {
    Buffer buffer(4);

    buffer.fill(9);

    CHECK(std::all_of(buffer.begin(), buffer.end(),
                      [](int n) { return n == 9; }));
}

void new_buffer_is_zero_initialized() {
    Buffer buffer(5);

    // Replaces the manual `for (i...) if (at(i) != 0)` loop from the
    // week 2 test_construccion.cpp.
    CHECK(std::all_of(buffer.begin(), buffer.end(),
                      [](int n) { return n == 0; }));
}

void iota_and_accumulate() {
    Buffer buffer(5);

    std::iota(buffer.begin(), buffer.end(), 1); // 1, 2, 3, 4, 5

    CHECK(std::accumulate(buffer.begin(), buffer.end(), 0) == 15);
}

void accumulate_on_empty_returns_initial_value() {
    Buffer empty(0);

    CHECK(std::accumulate(empty.begin(), empty.end(), 100) == 100);
}

void max_element_and_count_if() {
    Buffer buffer = make_buffer({12, 7, 45, 3, 28});

    CHECK(*std::max_element(buffer.begin(), buffer.end()) == 45);
    CHECK(std::count_if(buffer.begin(), buffer.end(),
                        [](int n) { return n % 2 == 0; }) == 2);
}

void max_element_on_empty_returns_end() {
    Buffer empty(0);

    CHECK(std::max_element(empty.begin(), empty.end()) == empty.end());
}

void find_if_found_and_not_found() {
    Buffer buffer = make_buffer({12, 7, 45, 3, 28});

    auto found = std::find_if(buffer.begin(), buffer.end(),
                              [](int n) { return n > 40; });
    auto missing = std::find_if(buffer.begin(), buffer.end(),
                                [](int n) { return n > 100; });

    CHECK(found != buffer.end());
    CHECK(found - buffer.begin() == 2);
    CHECK(missing == buffer.end());
}

void transform_into_another_buffer() {
    Buffer source = make_buffer({1, 2, 3});
    Buffer doubled(source.size());

    std::transform(source.begin(), source.end(), doubled.begin(),
                   [](int n) { return n * 2; });

    CHECK(doubled == make_buffer({2, 4, 6}));
    CHECK(source == make_buffer({1, 2, 3})); // the source is untouched
}

void sort_both_directions() {
    Buffer buffer = make_buffer({5, 1, 4, 2, 3});

    std::sort(buffer.begin(), buffer.end());
    CHECK(buffer == make_buffer({1, 2, 3, 4, 5}));

    std::sort(buffer.begin(), buffer.end(), std::greater<>());
    CHECK(buffer == make_buffer({5, 4, 3, 2, 1}));
}

void equality_compares_contents_not_addresses() {
    Buffer original = make_buffer({1, 2, 3});
    Buffer copy(original);

    CHECK(copy == original);
    CHECK(copy.data() != original.data()); // still a deep copy

    copy.at(2) = 99;
    CHECK(copy != original);

    CHECK(make_buffer({1, 2}) != make_buffer({1, 2, 3})); // different size
    CHECK(Buffer(0) == Buffer(0));
}

int main() {
    fill_sets_every_element();
    new_buffer_is_zero_initialized();
    iota_and_accumulate();
    accumulate_on_empty_returns_initial_value();
    max_element_and_count_if();
    max_element_on_empty_returns_end();
    find_if_found_and_not_found();
    transform_into_another_buffer();
    sort_both_directions();
    equality_compares_contents_not_addresses();
    return check::report("Buffer + <algorithm>");
}
