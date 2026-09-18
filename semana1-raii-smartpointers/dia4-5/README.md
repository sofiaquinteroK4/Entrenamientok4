# RAII Buffer Wrapper (Week 1, Day 4-5)

A small C++ exercise that wraps a raw heap buffer in a class following the RAII (Resource Acquisition Is Initialization) pattern: the constructor allocates memory, and the destructor releases it automatically.

## What it demonstrates

- **RAII**: memory is tied to an object's lifetime, so it's freed automatically when the object goes out of scope — no manual `delete` calls needed.
- **No copying**: the copy constructor and copy assignment operator are deleted, preventing two objects from owning (and double-freeing) the same buffer.
- **Move semantics**: ownership can be transferred between objects via move construction/assignment, similar to how `std::unique_ptr` works internally.
- **Exception safety**: the destructor still runs even if an exception is thrown while the object is alive, avoiding leaks.

## Files

- `raii_buffer.cpp` — the `RaiiBuffer` class plus a `main()` with three test scenarios (basic usage, move semantics, exception handling).

## Build and run

```bash
g++ -std=c++17 -Wall -Wextra -o raii_buffer raii_buffer.cpp
./raii_buffer
```

## Validate with Valgrind (Linux)

```bash
g++ -std=c++17 -g -o raii_buffer raii_buffer.cpp
valgrind --leak-check=full ./raii_buffer
```

Confirm the output ends with `All heap blocks were freed -- no leaks are possible`.