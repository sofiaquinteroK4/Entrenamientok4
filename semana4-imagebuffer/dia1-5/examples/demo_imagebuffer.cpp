#include "ImageBuffer.hpp"

#include <algorithm>
#include <iostream>
#include <utility>

// ---------------------------------------------------------------------
// Demo: ImageBuffer in action. Automated tests with assertions live in
// tests/; this program is for looking at what happens.
// ---------------------------------------------------------------------

void describe(const char* label, const ImageBuffer& image) {
    std::cout << "  " << label << ": " << image.width() << " x "
              << image.height() << " x " << image.channels() << " ("
              << image.size() << " bytes) at "
              << static_cast<const void*>(image.data()) << "\n";
}

// Draws a grayscale image as ASCII art: darker values -> sparser chars.
void draw(const ImageBuffer& image) {
    const char ramp[] = " .:-=+*#%@";
    constexpr std::size_t levels = sizeof(ramp) - 1;
    for (std::size_t y = 0; y < image.height(); ++y) {
        std::cout << "    ";
        for (std::size_t x = 0; x < image.width(); ++x) {
            std::cout << ramp[image.at(x, y, 0) * levels / 256];
        }
        std::cout << "\n";
    }
}

void demo_construction() {
    std::cout << "\n--- Construction: dimensions + zero-initialized pixels ---\n";
    ImageBuffer rgb(4, 3, 3);
    describe("rgb", rgb);
    std::cout << "  all zero: " << std::boolalpha
              << std::all_of(rgb.begin(), rgb.end(),
                             [](ImageBuffer::value_type v) { return v == 0; })
              << "\n";

    rgb.at(2, 1, 0) = 255; // pixel (2, 1), red channel
    std::cout << "  at(2, 1, R) = 255 lives at data()[(1 * 4 + 2) * 3 + 0] = "
                 "data()[18] -> "
              << static_cast<int>(rgb.data()[18]) << "\n";

    try {
        ImageBuffer bad(2, 2, 5);
    } catch (const std::invalid_argument& e) {
        std::cout << "  ImageBuffer(2, 2, 5) rejected: " << e.what() << "\n";
    }
}

void demo_gradient() {
    std::cout << "\n--- A 24 x 6 grayscale gradient ---\n";
    ImageBuffer gray(24, 6, 1);
    for (std::size_t y = 0; y < gray.height(); ++y) {
        for (std::size_t x = 0; x < gray.width(); ++x) {
            gray.at(x, y, 0) = static_cast<ImageBuffer::value_type>(
                x * 255 / (gray.width() - 1));
        }
    }
    draw(gray);
}

void demo_copy() {
    std::cout << "\n--- Copy: same pixels, different memory ---\n";
    ImageBuffer original(8, 3, 1);
    original.fill(200);
    ImageBuffer copy(original);

    describe("original", original);
    describe("copy    ", copy);

    copy.fill(100); // must not affect 'original'
    std::cout << "  after copy.fill(100):\n    original:\n";
    draw(original);
    std::cout << "    copy:\n";
    draw(copy);
}

void demo_move() {
    std::cout << "\n--- Move: same memory, source left empty ---\n";
    ImageBuffer source(640, 480, 4);
    describe("source before", source);

    ImageBuffer destination(std::move(source));
    describe("destination  ", destination);
    describe("source after ", source);
    std::cout << "  destination took the same address; 1.2 MB were NOT "
                 "copied.\n";
}

int main() {
    demo_construction();
    demo_gradient();
    demo_copy();
    demo_move();
    std::cout << "\nProgram finished. To verify there are no leaks, run it under "
                 "valgrind (Linux) or `leaks --atExit --` (macOS), or use "
                 "`ctest -L memcheck`.\n";
    return 0;
}
