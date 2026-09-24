#pragma once

// =====================================================================
// Week 1, Day 4-5: RAII applied to a raw heap buffer.
// =====================================================================
//
// The class acquires the resource in the constructor and releases it
// in the destructor (RAII). It only allows MOVING, not copying: two
// objects can never point at the same block of memory, so there's no
// risk of a double `delete`.
//
// The console trace (RAII_TRACE) is optional: it's only enabled in the
// demo executable, so the class stays decoupled from <iostream> and
// doesn't clutter the test output.

#include <cstddef>   // std::size_t
#include <utility>   // std::move, std::exchange
#include <stdexcept>

#ifdef RAII_TRACE
#include <iostream>
#endif

class RaiiBuffer {
public:
    // --- Constructor: this is the "A" in RAII (Acquisition) ---
    explicit RaiiBuffer(std::size_t size)
        : size_(size), data_(size_ > 0 ? new int[size_]() : nullptr)
    {
#ifdef RAII_TRACE
        std::cout << "[RaiiBuffer] Constructor: allocating "
                  << size_ << " ints (" << size_ * sizeof(int)
                  << " bytes) at " << data_ << "\n";
#endif
    }

    // --- Destructor: this is the "L" in RAII (Liberation) ---
    // This destructor always runs when the object goes out of scope,
    // with no need to remember to call delete[] ourselves.
    ~RaiiBuffer() {
#ifdef RAII_TRACE
        if (data_ != nullptr) {
            std::cout << "[RaiiBuffer] Destructor: freeing "
                      << size_ << " ints at " << data_ << "\n";
        } else {
            std::cout << "[RaiiBuffer] Destructor: nothing to free "
                      << "(the buffer was moved to another object)\n";
        }
#endif
        delete[] data_;
    }

    // --- COPYING is forbidden (would duplicate ownership) ---
    RaiiBuffer(const RaiiBuffer&) = delete;
    RaiiBuffer& operator=(const RaiiBuffer&) = delete;

    // --- MOVING is allowed (ownership transfer) ---
    // Moving means: "the new object takes the pointer, and the old
    // object is left as nullptr (empty, nothing to free)". This is
    // exactly what unique_ptr does internally.
    RaiiBuffer(RaiiBuffer&& other) noexcept
        : size_(other.size_),
          data_(std::exchange(other.data_, nullptr)) // steal the pointer
    {
        other.size_ = 0;
#ifdef RAII_TRACE
        std::cout << "[RaiiBuffer] Move constructor: ownership transferred\n";
#endif
    }

    RaiiBuffer& operator=(RaiiBuffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;                     // free what we already had
            data_ = std::exchange(other.data_, nullptr);
            size_ = other.size_;
            other.size_ = 0;
#ifdef RAII_TRACE
            std::cout << "[RaiiBuffer] Move assignment: ownership transferred\n";
#endif
        }
        return *this;
    }

    // --- Safe access to the data ---
    int& at(std::size_t index) {
        if (index >= size_) {
            throw std::out_of_range("Index out of range in RaiiBuffer::at");
        }
        return data_[index];
    }

    std::size_t size() const { return size_; }

private:
    std::size_t size_;
    int* data_;
};
