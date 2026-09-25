#include "Buffer.hpp"
#include "check.hpp"

#include <type_traits>
#include <utility>
#include <vector>

// Regression tests: each one pins down a behavior that is easy to break
// by accident while refactoring (e.g. when replacing a loop with an
// algorithm, or "simplifying" a special member). If one of them fails,
// something that used to work stopped working.

void copying_an_empty_buffer() {
    // std::copy over an empty range must not touch the null pointer.
    Buffer empty(0);

    Buffer copy(empty);

    CHECK(copy.empty());
    CHECK(copy.data() == nullptr);
}

void copying_a_moved_from_buffer() {
    Buffer source(3);
    Buffer destination(std::move(source));

    Buffer copy(source); // 'source' is empty but still valid

    CHECK(copy.empty());
    CHECK(destination.size() == 3);
}

void self_move_assignment_keeps_the_data() {
    Buffer buffer(2);
    buffer.fill(5);

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#endif
    buffer = std::move(buffer); // without `this != &other`, this frees the data
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    CHECK(buffer.size() == 2);
    CHECK(buffer.at(0) == 5 && buffer.at(1) == 5);
}

void self_copy_assignment_keeps_the_data() {
    Buffer buffer(2);
    buffer.fill(8);

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
    buffer = buffer;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    CHECK(buffer.size() == 2);
    CHECK(buffer.at(0) == 8 && buffer.at(1) == 8);
}

void swap_exchanges_without_copying() {
    Buffer a(1);
    Buffer b(3);
    const int* a_data = a.data();
    const int* b_data = b.data();

    using std::swap;
    swap(a, b);

    CHECK(a.data() == b_data && a.size() == 3);
    CHECK(b.data() == a_data && b.size() == 1);
}

void vector_reallocation_moves_instead_of_copying() {
    // If someone drops `noexcept` from the move constructor, std::vector
    // falls back to COPYING on reallocation (slower, and new addresses).
    static_assert(std::is_nothrow_move_constructible_v<Buffer>,
                  "Buffer's move constructor must be noexcept");
    static_assert(std::is_nothrow_move_assignable_v<Buffer>,
                  "Buffer's move assignment must be noexcept");

    std::vector<Buffer> buffers;
    buffers.reserve(1);
    buffers.emplace_back(4);
    const int* first_data = buffers[0].data();

    buffers.emplace_back(4); // exceeds capacity -> reallocation

    CHECK(buffers[0].data() == first_data); // moved, not copied
}

int main() {
    copying_an_empty_buffer();
    copying_a_moved_from_buffer();
    self_move_assignment_keeps_the_data();
    self_copy_assignment_keeps_the_data();
    swap_exchanges_without_copying();
    vector_reallocation_moves_instead_of_copying();
    return check::report("Buffer regressions");
}
