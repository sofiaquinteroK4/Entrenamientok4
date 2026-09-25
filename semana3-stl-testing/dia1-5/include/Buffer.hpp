#pragma once

// =====================================================================
// Week 3: idiomatic STL on top of the week 2 Buffer.
// =====================================================================
//
// Same Rule of 5 as week 2 (see
// semana2-regla-de-cinco/dia1-5/include/Buffer.hpp for the theory).
// What's new is begin()/end(): that's all <algorithm> needs to work on
// a Buffer, and it also enables range-based for loops. A raw pointer
// already is a random-access iterator, so there's no need to write an
// iterator class: begin() returns data_ and end() returns
// data_ + size_ (one past the last element).
//
// The special members that used to copy element by element now
// delegate to <algorithm> (std::copy, std::fill, std::equal).

#include <algorithm> // std::copy, std::fill, std::equal
#include <cstddef>   // std::size_t
#include <stdexcept>
#include <utility>   // std::exchange, std::swap

class Buffer {
public:
    // The same aliases the standard containers expose, so generic code
    // can ask a Buffer for its element and iterator types.
    using value_type = int;
    using iterator = int*;
    using const_iterator = const int*;

    // --- Constructor: resource acquisition (RAII), zero-initialized ---
    explicit Buffer(std::size_t size)
        : size_(size), data_(size_ > 0 ? new int[size_]() : nullptr) {}

    // --- Destructor: resource release ---
    ~Buffer() { delete[] data_; }

    // --- Copy constructor: DEEP copy via std::copy ---
    // With size 0 the range [begin(), end()) is empty and std::copy
    // does nothing, so there's no need for a special case.
    Buffer(const Buffer& other)
        : size_(other.size_),
          data_(other.size_ > 0 ? new int[other.size_] : nullptr)
    {
        std::copy(other.begin(), other.end(), begin());
    }

    // --- Copy assignment operator: copy-and-swap ---
    Buffer& operator=(const Buffer& other) {
        Buffer tmp(other);
        swap(tmp);
        return *this;
    }

    void swap(Buffer& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);
    }

    // Free swap found via ADL, so `using std::swap; swap(a, b);` picks
    // the cheap member swap instead of the generic three-move version.
    friend void swap(Buffer& a, Buffer& b) noexcept { a.swap(b); }

    // --- Move constructor / move assignment: transfer the pointer ---
    // They must stay noexcept: std::vector<Buffer> only moves its
    // elements on reallocation when the move constructor can't throw
    // (otherwise it copies them). tests/test_regresion.cpp guards this.
    Buffer(Buffer&& other) noexcept
        : size_(std::exchange(other.size_, 0)),
          data_(std::exchange(other.data_, nullptr)) {}

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = std::exchange(other.data_, nullptr);
            size_ = std::exchange(other.size_, 0);
        }
        return *this;
    }

    // --- Iterators: the bridge to <algorithm> ---
    iterator begin() { return data_; }
    iterator end() { return data_ + size_; }
    const_iterator begin() const { return data_; }
    const_iterator end() const { return data_ + size_; }

    // --- Element access (const-correct, as in week 2) ---
    int& at(std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range in Buffer::at");
        }
        return data_[index];
    }

    const int& at(std::size_t index) const {
        if (index >= size_) {
            throw std::out_of_range("Index out of range in Buffer::at");
        }
        return data_[index];
    }

    // Sets every element to `value`. Replaces a manual indexed loop.
    void fill(int value) { std::fill(begin(), end(), value); }

    std::size_t size() const { return size_; }
    bool empty() const { return size_ == 0; }

    // Same name as std::vector::data(). Used by the tests to compare
    // addresses (deep copy vs move).
    const int* data() const { return data_; }

private:
    std::size_t size_;
    int* data_;
};

// Two buffers are equal if they have the same size and the same
// elements, not the same address. The four-iterator overload of
// std::equal already returns false when the lengths differ.
inline bool operator==(const Buffer& a, const Buffer& b) {
    return std::equal(a.begin(), a.end(), b.begin(), b.end());
}

inline bool operator!=(const Buffer& a, const Buffer& b) {
    return !(a == b);
}
