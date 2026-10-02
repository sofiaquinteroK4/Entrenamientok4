#include "ImageBuffer.hpp"
#include "alloc_counter.hpp"
#include "check.hpp"

#include <utility>

// Test: empty images (edge case). An image can be empty in three ways:
// default-constructed, with a 0 width or 0 height, or moved-from. None
// of them owns memory, and every operation must still be safe on them.

void default_constructed_is_empty() {
    ImageBuffer image;

    CHECK(image.empty());
    CHECK(image.width() == 0 && image.height() == 0 && image.channels() == 0);
    CHECK(image.size() == 0);
    CHECK(image.data() == nullptr);
    CHECK(image.begin() == image.end());
}

void zero_width_or_height_is_empty_but_keeps_channels() {
    ImageBuffer no_width(0, 5, 3);
    ImageBuffer no_height(5, 0, 3);

    CHECK(no_width.empty() && no_width.data() == nullptr);
    CHECK(no_height.empty() && no_height.data() == nullptr);
    CHECK(no_width.channels() == 3); // valid image, just no pixels
    CHECK_THROWS(no_width.at(0, 0, 0), std::out_of_range);
}

void operations_on_empty_are_safe() {
    ImageBuffer image;

    image.fill(255); // empty range: does nothing

    CHECK_THROWS(image.at(0, 0, 0), std::out_of_range);
    CHECK(image == ImageBuffer());
}

void copying_an_empty_image() {
    ImageBuffer empty(0, 4, 3);

    ImageBuffer copy(empty);
    ImageBuffer assigned(2, 2, 1);
    assigned = empty;

    CHECK(copy.empty() && copy.data() == nullptr);
    CHECK(copy == empty); // same dimensions, including the 3 channels
    CHECK(assigned.empty() && assigned == empty);
}

void moving_an_empty_image() {
    ImageBuffer empty;

    ImageBuffer moved(std::move(empty));

    CHECK(moved.empty());
    CHECK(empty.empty());
}

void empty_images_compare_by_dimensions() {
    // Same size() (0) but different dimensions: not the same image.
    CHECK(ImageBuffer(0, 4, 3) != ImageBuffer(4, 0, 3));
    CHECK(ImageBuffer(0, 4, 3) == ImageBuffer(0, 4, 3));
}

int main() {
    default_constructed_is_empty();
    zero_width_or_height_is_empty_but_keeps_channels();
    operations_on_empty_are_safe();
    copying_an_empty_image();
    moving_an_empty_image();
    empty_images_compare_by_dimensions();
    // Every block allocated by the tests above must have been freed.
    CHECK(alloc_counter::live_arrays == 0);
    return check::report("ImageBuffer empty images");
}
