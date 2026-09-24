#pragma once

// Day 2: custom deleters for unique_ptr.
//
// RawHandle simulates a resource from a C-style API (e.g. a file
// descriptor or a system handle) that is NOT released with `delete`,
// but with its own "close" function. HandleDeleter is the functor that
// unique_ptr invokes instead of the default delete.

struct RawHandle {
    int id;
};

class HandleDeleter {
public:
    // `close_count`, if not nullptr, is incremented every time the
    // deleter runs. It only exists so the tests can verify that the
    // custom deleter ran (and how many times).
    explicit HandleDeleter(int* close_count = nullptr) : close_count_(close_count) {}

    void operator()(RawHandle* handle) const {
        if (close_count_ != nullptr) {
            ++(*close_count_);
        }
        // In a real scenario this would be something like
        // close_handle(handle->id) instead of delete; for this
        // exercise RawHandle is still a heap object, so we do free it
        // with delete.
        delete handle;
    }

private:
    int* close_count_;
};
