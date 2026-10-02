#include "ImageBuffer.hpp"
#include "alloc_counter.hpp"
#include "check.hpp"

#include <algorithm>
#include <limits>
#include <utility> // std::as_const

// Test: construction, dimension validation, zero-init, memory layout,
// and bounds-checked access (Day 2: RAII + basic accessors).

void dimensions_and_size() {
    ImageBuffer image(4, 3, 3); // 4 x 3 RGB

    CHECK(image.width() == 4);
    CHECK(image.height() == 3);
    CHECK(image.channels() == 3);
    CHECK(image.size() == 4 * 3 * 3);
    CHECK(!image.empty());
    CHECK(image.data() != nullptr);
}

void new_image_is_zero_initialized() {
    ImageBuffer image(5, 2, 4);

    CHECK(std::all_of(image.begin(), image.end(),
                      [](ImageBuffer::value_type v) { return v == 0; }));
}

void channels_must_be_between_1_and_4() {
    CHECK_THROWS(ImageBuffer(2, 2, 0), std::invalid_argument);
    CHECK_THROWS(ImageBuffer(2, 2, 5), std::invalid_argument);

    // The valid limits must not throw.
    CHECK(ImageBuffer(2, 2, 1).channels() == 1);
    CHECK(ImageBuffer(2, 2, 4).channels() == 4);
}

void size_overflow_is_rejected_before_allocating() {
    constexpr std::size_t max = std::numeric_limits<std::size_t>::max();

    const std::size_t live_before = alloc_counter::live_arrays;

    CHECK_THROWS(ImageBuffer(max, 2, 1), std::length_error); // w * h
    CHECK_THROWS(ImageBuffer(max / 2 + 1, 1, 2), std::length_error); // * c
    CHECK_THROWS(ImageBuffer(4, 4, 0), std::invalid_argument);

    CHECK(alloc_counter::live_arrays == live_before); // nothing allocated
}

void layout_is_row_major_and_interleaved() {
    ImageBuffer image(3, 2, 3);

    image.at(2, 1, 0) = 10; // pixel (x=2, y=1), channel R
    image.at(2, 1, 2) = 30; // same pixel, channel B

    // (y * width + x) * channels + c = (1 * 3 + 2) * 3 + c = 15 + c
    CHECK(image.data()[15] == 10);
    CHECK(image.data()[16] == 0);
    CHECK(image.data()[17] == 30);
}

void at_checks_every_coordinate() {
    ImageBuffer image(3, 2, 3);

    CHECK_THROWS(image.at(3, 0, 0), std::out_of_range); // x == width
    CHECK_THROWS(image.at(0, 2, 0), std::out_of_range); // y == height
    CHECK_THROWS(image.at(0, 0, 3), std::out_of_range); // channel == channels
    CHECK_THROWS(std::as_const(image).at(3, 0, 0), std::out_of_range);

    // The last valid pixel/channel must not throw.
    image.at(2, 1, 2) = 7;
    CHECK(std::as_const(image).at(2, 1, 2) == 7);
}

void fill_sets_every_value() {
    ImageBuffer image(2, 2, 3);

    image.fill(200);

    CHECK(std::all_of(image.begin(), image.end(),
                      [](ImageBuffer::value_type v) { return v == 200; }));
}

int main() {
    dimensions_and_size();
    new_image_is_zero_initialized();
    channels_must_be_between_1_and_4();
    size_overflow_is_rejected_before_allocating();
    layout_is_row_major_and_interleaved();
    at_checks_every_coordinate();
    fill_sets_every_value();
    // Every block allocated by the tests above must have been freed.
    CHECK(alloc_counter::live_arrays == 0);
    return check::report("ImageBuffer construction and access");
}
