#pragma once

// =====================================================================
// Week 4: ImageBuffer — consolidation of Month 1.
// =====================================================================
//
// An image stored as one contiguous heap block of 8-bit values:
// width x height pixels, each with `channels` values (1 = grayscale,
// 3 = RGB, 4 = RGBA). It brings together everything from the month:
//
// - RAII (week 1): the constructor allocates, the destructor frees.
// - Rule of 5 (week 2): deep copy, move without allocating,
//   const-correct access.
// - Idiomatic STL (week 3): begin()/end() so <algorithm> works on the
//   pixel data; the special members use std::copy/std::fill/std::equal.
//
// Memory layout: row-major and interleaved. Row 0 comes first, then
// row 1, and so on; inside a row, the channels of one pixel are stored
// next to each other (R G B R G B ...). The value for pixel (x, y),
// channel c lives at index (y * width + x) * channels + c. This is the
// layout most image libraries (stb_image, OpenCV's Mat, PNG decoders)
// hand out, so data() can be passed straight to them.
//
// Note: this class manages its memory by hand on purpose, to practice
// the Rule of 5. Production code would store a std::vector<uint8_t>
// and get all five special members for free (Rule of 0).

#include <algorithm> // std::copy, std::fill, std::equal
#include <cstddef>   // std::size_t
#include <cstdint>   // std::uint8_t
#include <limits>
#include <stdexcept>
#include <utility>   // std::exchange, std::swap

class ImageBuffer {
public:
    using value_type = std::uint8_t;
    using iterator = value_type*;
    using const_iterator = const value_type*;

    static constexpr std::size_t max_channels = 4;

    // --- Empty image: 0 x 0 with 0 channels, owns no memory ---
    // It's also the state a moved-from ImageBuffer is left in.
    ImageBuffer() noexcept = default;

    // --- Constructor: resource acquisition (RAII), zero-initialized ---
    // Throws std::invalid_argument if `channels` isn't in [1, 4], and
    // std::length_error if width * height * channels doesn't fit in a
    // std::size_t. Both checks run BEFORE allocating, so a rejected
    // image never leaks. A 0-wide or 0-tall image is valid and empty.
    //
    // data_ is computed from the parameters, not from the members
    // width_/height_/channels_, so it doesn't depend on the order in
    // which the members are declared.
    ImageBuffer(std::size_t width, std::size_t height, std::size_t channels)
        : width_(width),
          height_(height),
          channels_(channels),
          data_(allocate(checked_size(width, height, channels))) {}

    // --- Destructor: resource release ---
    ~ImageBuffer() { delete[] data_; }

    // --- Copy constructor: DEEP copy ---
    // Same dimensions, same pixels, but its own block of memory.
    ImageBuffer(const ImageBuffer& other)
        : width_(other.width_),
          height_(other.height_),
          channels_(other.channels_),
          data_(allocate(other.size()))
    {
        std::copy(other.begin(), other.end(), data_);
    }

    // --- Copy assignment: copy-and-swap ---
    // If the copy throws (std::bad_alloc), *this is left untouched
    // (strong guarantee). Self-assignment needs no special check.
    ImageBuffer& operator=(const ImageBuffer& other) {
        ImageBuffer tmp(other);
        swap(tmp);
        return *this;
    }

    // --- Move constructor: steals the pointer and the dimensions ---
    // No allocation; the source is left as an empty 0 x 0 x 0 image.
    ImageBuffer(ImageBuffer&& other) noexcept
        : width_(std::exchange(other.width_, 0)),
          height_(std::exchange(other.height_, 0)),
          channels_(std::exchange(other.channels_, 0)),
          data_(std::exchange(other.data_, nullptr)) {}

    // --- Move assignment: move-and-swap ---
    // `tmp` takes over other's image (leaving `other` empty), then
    // trades places with *this; our old pixels die with `tmp`. On
    // self-move, tmp takes our own image and the swap gives it right
    // back, so no `this != &other` check is needed.
    ImageBuffer& operator=(ImageBuffer&& other) noexcept {
        ImageBuffer tmp(std::move(other));
        swap(tmp);
        return *this;
    }

    void swap(ImageBuffer& other) noexcept {
        std::swap(width_, other.width_);
        std::swap(height_, other.height_);
        std::swap(channels_, other.channels_);
        std::swap(data_, other.data_);
    }

    friend void swap(ImageBuffer& a, ImageBuffer& b) noexcept { a.swap(b); }

    // --- Dimensions ---
    [[nodiscard]] std::size_t width() const noexcept { return width_; }
    [[nodiscard]] std::size_t height() const noexcept { return height_; }
    [[nodiscard]] std::size_t channels() const noexcept { return channels_; }

    // Total number of values (width * height * channels). With 8-bit
    // channels, it's also the size in bytes.
    [[nodiscard]] std::size_t size() const noexcept {
        return width_ * height_ * channels_;
    }

    [[nodiscard]] bool empty() const noexcept { return size() == 0; }

    // --- Raw access, for passing the pixels to C-style APIs ---
    [[nodiscard]] value_type* data() noexcept { return data_; }
    [[nodiscard]] const value_type* data() const noexcept { return data_; }

    // --- Iterators over all values, in memory order ---
    [[nodiscard]] iterator begin() noexcept { return data_; }
    [[nodiscard]] iterator end() noexcept { return data_ + size(); }
    [[nodiscard]] const_iterator begin() const noexcept { return data_; }
    [[nodiscard]] const_iterator end() const noexcept { return data_ + size(); }

    // --- Checked access to one channel of one pixel ---
    // Throws std::out_of_range if x, y or channel is outside the image.
    [[nodiscard]] value_type& at(std::size_t x, std::size_t y, std::size_t channel) {
        check_bounds(x, y, channel);
        return data_[index(x, y, channel)];
    }

    [[nodiscard]] const value_type& at(std::size_t x, std::size_t y,
                                       std::size_t channel) const {
        check_bounds(x, y, channel);
        return data_[index(x, y, channel)];
    }

    // Sets every channel of every pixel to `value`.
    void fill(value_type value) noexcept { std::fill(begin(), end(), value); }

private:
    // Validates the dimensions and returns width * height * channels
    // without overflowing: each multiplication is checked against
    // SIZE_MAX first by dividing instead of multiplying.
    static std::size_t checked_size(std::size_t width, std::size_t height,
                                    std::size_t channels) {
        if (channels == 0 || channels > max_channels) {
            throw std::invalid_argument(
                "ImageBuffer: channels must be between 1 and 4");
        }
        constexpr std::size_t max = std::numeric_limits<std::size_t>::max();
        if (width != 0 && height > max / width) {
            throw std::length_error("ImageBuffer: width * height overflows");
        }
        const std::size_t pixels = width * height;
        if (pixels != 0 && channels > max / pixels) {
            throw std::length_error(
                "ImageBuffer: width * height * channels overflows");
        }
        return pixels * channels;
    }

    static value_type* allocate(std::size_t size) {
        return size > 0 ? new value_type[size]() : nullptr;
    }

    void check_bounds(std::size_t x, std::size_t y, std::size_t channel) const {
        if (x >= width_ || y >= height_ || channel >= channels_) {
            throw std::out_of_range("ImageBuffer::at: pixel or channel out of range");
        }
    }

    [[nodiscard]] std::size_t index(std::size_t x, std::size_t y,
                                    std::size_t channel) const noexcept {
        return (y * width_ + x) * channels_ + channel;
    }

    std::size_t width_ = 0;
    std::size_t height_ = 0;
    std::size_t channels_ = 0;
    value_type* data_ = nullptr;
};

// Two images are equal if they have the same dimensions and the same
// pixel values (not the same address). Same size() isn't enough: a
// 2 x 3 and a 3 x 2 image have the same number of values.
[[nodiscard]] inline bool operator==(const ImageBuffer& a, const ImageBuffer& b) {
    return a.width() == b.width() && a.height() == b.height() &&
           a.channels() == b.channels() &&
           std::equal(a.begin(), a.end(), b.begin(), b.end());
}

[[nodiscard]] inline bool operator!=(const ImageBuffer& a, const ImageBuffer& b) {
    return !(a == b);
}
