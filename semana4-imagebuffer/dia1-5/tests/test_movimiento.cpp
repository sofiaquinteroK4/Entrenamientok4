#include "ImageBuffer.hpp"
#include "alloc_counter.hpp"
#include "check.hpp"

#include <type_traits>
#include <utility>
#include <vector>

// Test: state after a move (Day 3). The destination takes the same
// block of memory (no allocation), and the source is left as a valid,
// empty 0 x 0 x 0 image that can be destroyed or reused.

void move_constructor_transfers_the_block() {
    ImageBuffer source(4, 3, 3);
    source.at(1, 2, 1) = 77;
    const ImageBuffer::value_type* block = source.data();

    ImageBuffer destination(std::move(source));

    CHECK(destination.data() == block); // same address: nothing copied
    CHECK(destination.width() == 4 && destination.height() == 3 &&
          destination.channels() == 3);
    CHECK(destination.at(1, 2, 1) == 77);
}

void moved_from_source_is_empty() {
    ImageBuffer source(4, 3, 3);

    ImageBuffer destination(std::move(source));

    // Every dimension goes back to 0, not just the pointer: otherwise
    // width()/height() would describe pixels that no longer exist.
    CHECK(source.empty());
    CHECK(source.width() == 0 && source.height() == 0 && source.channels() == 0);
    CHECK(source.data() == nullptr);
    CHECK(source.begin() == source.end());
    CHECK(source == ImageBuffer());
    CHECK_THROWS(source.at(0, 0, 0), std::out_of_range);
}

void move_assignment_transfers_and_frees_the_old_block() {
    ImageBuffer source(2, 2, 4);
    source.fill(50);
    const ImageBuffer::value_type* block = source.data();
    ImageBuffer target(8, 8, 1);
    const std::size_t live_before = alloc_counter::live_arrays; // 2 blocks

    target = std::move(source);

    // target's old 8 x 8 block is freed right away, and no new block is
    // allocated: one block fewer than before.
    CHECK(alloc_counter::live_arrays == live_before - 1);
    CHECK(target.data() == block);
    CHECK(target.width() == 2 && target.height() == 2 && target.channels() == 4);
    CHECK(target.at(1, 1, 3) == 50);
    CHECK(source.empty() && source.data() == nullptr);
}

void moved_from_image_can_be_reused() {
    ImageBuffer source(2, 2, 1);
    ImageBuffer destination(std::move(source));

    source = ImageBuffer(3, 1, 3); // assign a new image to it
    source.at(2, 0, 2) = 9;

    CHECK(source.width() == 3 && source.at(2, 0, 2) == 9);
    CHECK(destination.width() == 2); // unaffected
}

void self_move_assignment_keeps_the_image() {
    ImageBuffer image(2, 2, 3);
    image.fill(12);
    const ImageBuffer::value_type* block = image.data();

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-move"
#endif
    image = std::move(image);
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    CHECK(image.data() == block);
    CHECK(image.width() == 2 && image.height() == 2 && image.channels() == 3);
    CHECK(image.at(1, 1, 2) == 12);
}

void swap_exchanges_without_copying() {
    ImageBuffer a(1, 1, 1);
    ImageBuffer b(4, 4, 4);
    const ImageBuffer::value_type* a_block = a.data();
    const ImageBuffer::value_type* b_block = b.data();

    using std::swap;
    swap(a, b);

    CHECK(a.data() == b_block && a.width() == 4);
    CHECK(b.data() == a_block && b.width() == 1);
}

void vector_reallocation_moves_instead_of_copying() {
    static_assert(std::is_nothrow_move_constructible_v<ImageBuffer>,
                  "ImageBuffer's move constructor must be noexcept");
    static_assert(std::is_nothrow_move_assignable_v<ImageBuffer>,
                  "ImageBuffer's move assignment must be noexcept");

    std::vector<ImageBuffer> frames;
    frames.reserve(1);
    frames.emplace_back(4, 4, 3);
    const ImageBuffer::value_type* first_block = frames[0].data();

    frames.emplace_back(4, 4, 3); // exceeds capacity -> reallocation

    CHECK(frames[0].data() == first_block); // moved, not copied
}

int main() {
    move_constructor_transfers_the_block();
    moved_from_source_is_empty();
    move_assignment_transfers_and_frees_the_old_block();
    moved_from_image_can_be_reused();
    self_move_assignment_keeps_the_image();
    swap_exchanges_without_copying();
    vector_reallocation_moves_instead_of_copying();
    // Every block allocated by the tests above must have been freed.
    CHECK(alloc_counter::live_arrays == 0);
    return check::report("ImageBuffer state after move");
}
