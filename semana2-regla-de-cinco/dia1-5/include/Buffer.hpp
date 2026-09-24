#pragma once

// =====================================================================
// Week 2: const-correctness + Rule of 3/5/0 + deep copy + move
// =====================================================================
//
// Picks up RaiiBuffer from week 1 (which only allowed moving, not
// copying) and evolves it into a class that CAN be copied (with a deep
// copy of the resource) and CAN be moved (transferring ownership
// without duplicating memory).
//
// The console trace (BUFFER_TRACE) is optional: it's only enabled in
// the demo executable (examples/demo_semana2.cpp), so the class stays
// decoupled from <iostream> and doesn't clutter the test output.
//
// Note: this class exists for teaching purposes, to practice the Rule
// of 5 by hand. Production code would prefer the Rule of 0 using
// std::unique_ptr<int[]>.
//
// ---------------------------------------------------------------------
// Rule of 3 / 5 / 0 (theory)
// ---------------------------------------------------------------------
// - Rule of 3: if a class manages a resource manually (heap, file
//   handle, etc.) and therefore needs a custom destructor, it normally
//   also needs the copy constructor and the copy assignment operator.
//   If you don't define them, the compiler generates default ones that
//   copy the member pointer as-is (shallow copy), and two objects end
//   up pointing at the same block -> double `delete`.
// - Rule of 5: in C++11+, the move constructor and move assignment
//   operator are added to the list. If you manage a resource, you
//   normally want to define all 5 (destructor, copy x2, move x2) or
//   none of them.
// - Rule of 0: delegating the resource to a member that already
//   manages itself (std::vector, std::unique_ptr, std::string...)
//   avoids having to write any of the 5 special members.

#include <algorithm> // std::copy
#include <cstddef>   // std::size_t
#include <stdexcept>
#include <utility>   // std::move, std::exchange, std::swap

#ifdef BUFFER_TRACE
#include <iostream>
#include <string>
#endif

class Buffer {
public:
    // --- Constructor: resource acquisition (RAII) ---
    explicit Buffer(std::size_t size)
        : size_(size), data_(size_ > 0 ? new int[size_]() : nullptr)
    {
#ifdef BUFFER_TRACE
        std::cout << "[Buffer] Constructor: " << size_ << " ints at "
                  << static_cast<void*>(data_) << "\n";
#endif
    }

    // --- Destructor: resource release ---
    ~Buffer() {
#ifdef BUFFER_TRACE
        if (data_ != nullptr) {
            std::cout << "[Buffer] Destructor: freeing " << size_
                      << " ints at " << static_cast<void*>(data_) << "\n";
        } else {
            std::cout << "[Buffer] Destructor: nothing to free "
                         "(empty or moved)\n";
        }
#endif
        delete[] data_;
    }

    // --- Copy constructor: DEEP copy ---
    // Each object ends up with its own independent block of memory,
    // with the same size and contents, but at a different address.
    Buffer(const Buffer& other)
        : size_(other.size_),
          data_(other.size_ > 0 ? new int[other.size_] : nullptr)
    {
        if (other.size_ > 0) {
            std::copy(other.data_, other.data_ + other.size_, data_);
        }
#ifdef BUFFER_TRACE
        std::cout << "[Buffer] Copy constructor: deep copy from "
                  << static_cast<const void*>(other.data_) << " -> "
                  << static_cast<void*>(data_) << "\n";
#endif
    }

    // --- Copy assignment operator: copy-and-swap ---
    // Builds a temporary copy (reusing the copy constructor) and swaps
    // it into *this. If `Buffer tmp(other)` throws, *this is left
    // untouched (strong exception guarantee), and there's no need to
    // check for self-assignment by hand: if other == *this, tmp is an
    // independent copy and the swap is a functional no-op.
    Buffer& operator=(const Buffer& other) {
        Buffer tmp(other);
        swap(tmp);
#ifdef BUFFER_TRACE
        std::cout << "[Buffer] Copy assignment: deep copy into "
                  << static_cast<void*>(data_) << "\n";
#endif
        return *this;
    }

    void swap(Buffer& other) noexcept {
        std::swap(size_, other.size_);
        std::swap(data_, other.data_);
    }

    // --- Move constructor: transfers the pointer (steals the
    // resource) without duplicating memory. The source is left empty
    // (data_ == nullptr), so its destructor frees nothing.
    Buffer(Buffer&& other) noexcept
        : size_(other.size_),
          data_(std::exchange(other.data_, nullptr))
    {
        other.size_ = 0;
#ifdef BUFFER_TRACE
        std::cout << "[Buffer] Move constructor: ownership transferred "
                     "(no new allocation)\n";
#endif
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = std::exchange(other.data_, nullptr);
            size_ = other.size_;
            other.size_ = 0;
#ifdef BUFFER_TRACE
            std::cout << "[Buffer] Move assignment: ownership "
                         "transferred (no new allocation)\n";
#endif
        }
        return *this;
    }

    // --- const-correctness ---
    // `at()` has two overloads: one for mutable objects (allows
    // writing) and one for const objects (read-only). The compiler
    // picks automatically based on the constness of the calling object.
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

    std::size_t size() const { return size_; }

    const void* address() const { return data_; }

#ifdef BUFFER_TRACE
    void print(const std::string& label) const {
        std::cout << "  " << label << ": size=" << size_
                  << " data=" << address();
        if (size_ > 0) {
            std::cout << " [0]=" << data_[0];
        }
        std::cout << "\n";
    }
#endif

private:
    std::size_t size_;
    int* data_;
};
