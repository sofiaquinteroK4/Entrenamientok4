#include "ImageBuffer.hpp"
#include "alloc_counter.hpp"
#include "check.hpp"

// Test: independence after copy (Day 3). A copy has the same dimensions
// and pixels, but its own memory: changing one never changes the other.

// Builds a small image whose values are all different, so a copy that
// shuffled or dropped values would be caught.
ImageBuffer make_gradient(std::size_t width, std::size_t height,
                          std::size_t channels) {
    ImageBuffer image(width, height, channels);
    ImageBuffer::value_type next = 1;
    for (auto& value : image) {
        value = next++;
    }
    return image;
}

void copy_constructor_duplicates_the_pixels() {
    ImageBuffer original = make_gradient(3, 2, 3);

    ImageBuffer copy(original);

    CHECK(copy == original);
    CHECK(copy.data() != original.data()); // deep copy: new block
}

void modifying_the_copy_leaves_the_original_untouched() {
    ImageBuffer original = make_gradient(3, 2, 3);
    ImageBuffer copy(original);

    copy.at(1, 1, 0) = 250;
    copy.fill(9);

    CHECK(original == make_gradient(3, 2, 3));
}

void modifying_the_original_leaves_the_copy_untouched() {
    ImageBuffer original = make_gradient(2, 2, 1);
    ImageBuffer copy(original);

    original.fill(0);

    CHECK(copy == make_gradient(2, 2, 1));
}

void copy_assignment_takes_the_source_dimensions() {
    ImageBuffer source = make_gradient(4, 3, 4);
    ImageBuffer target(1, 1, 1);
    const std::size_t live_before = alloc_counter::live_arrays;

    target = source;

    // target's old block is freed and replaced by a new one: same count.
    CHECK(alloc_counter::live_arrays == live_before);
    CHECK(target.width() == 4 && target.height() == 3 && target.channels() == 4);
    CHECK(target == source);
    CHECK(target.data() != source.data());

    target.fill(0);
    CHECK(source == make_gradient(4, 3, 4));
}

void copy_assignment_can_shrink_the_image() {
    ImageBuffer small = make_gradient(1, 1, 3);
    ImageBuffer big = make_gradient(8, 8, 4);

    big = small;

    CHECK(big == small);
    CHECK(big.size() == 3);
}

void self_copy_assignment_keeps_the_image() {
    ImageBuffer image = make_gradient(2, 2, 3);

#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wself-assign-overloaded"
#endif
    image = image;
#if defined(__clang__)
#pragma clang diagnostic pop
#endif

    CHECK(image == make_gradient(2, 2, 3));
}

int main() {
    copy_constructor_duplicates_the_pixels();
    modifying_the_copy_leaves_the_original_untouched();
    modifying_the_original_leaves_the_copy_untouched();
    copy_assignment_takes_the_source_dimensions();
    copy_assignment_can_shrink_the_image();
    self_copy_assignment_keeps_the_image();
    // Every block allocated by the tests above must have been freed.
    CHECK(alloc_counter::live_arrays == 0);
    return check::report("ImageBuffer copy independence");
}
