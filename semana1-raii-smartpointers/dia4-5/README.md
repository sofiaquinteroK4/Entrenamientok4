# RAII Buffer Wrapper (Week 1, Day 4-5)

A small C++ exercise that wraps a raw heap buffer in a class following the RAII (Resource Acquisition Is Initialization) pattern: the constructor allocates memory, and the destructor releases it automatically.

## What it demonstrates

- **RAII**: memory is tied to an object's lifetime, so it's freed automatically when the object goes out of scope — no manual `delete` calls needed.
- **No copying**: the copy constructor and copy assignment operator are deleted, preventing two objects from owning (and double-freeing) the same buffer.
- **Move semantics**: ownership can be transferred between objects via move construction/assignment, similar to how `std::unique_ptr` works internally.
- **Exception safety**: the destructor still runs even if an exception is thrown while the object is alive, avoiding leaks.

## Files

- `include/RaiiBuffer.hpp` — the `RaiiBuffer` class (header-only, no `<iostream>` dependency unless `RAII_TRACE` is defined).
- `examples/demo_raii_buffer.cpp` — a `main()` with three demo scenarios (basic usage, move semantics, exception handling), compiled with console traces on.
- `tests/` — real tests with assertions and a non-zero exit code on failure, wired into CTest (`raii_construccion`, `raii_movimiento`).

## Build and run

From the repo root (see the [root README](../../README.md) for the full setup):

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure -R raii_
./build/semana1-raii-smartpointers/dia4-5/demo_raii_buffer
```

## Validate with sanitizers

```bash
cmake -S . -B build-asan -DENABLE_SANITIZERS=ON
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure -R raii_
```

Confirms there are no leaks (ASan) or undefined behavior (UBSan) across construction, move, and exception paths.