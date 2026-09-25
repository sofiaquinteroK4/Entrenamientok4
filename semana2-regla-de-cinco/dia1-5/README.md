# Rule of 3/5/0 + Deep Copy + Move (Week 2, Day 1-5)

Follow-up to the Week 1 exercise (`RaiiBuffer`, which only allowed
moving). Here `Buffer` implements the full Rule of 5: besides moving, it
can now also be **copied**, performing a **deep copy** of the heap
resource.

## Weekly plan

- **Day 1 — const-correctness**: methods that don't modify the object
  are marked `const` (`size()`, `address()`, `print()`), and `at()`
  has a `const` overload that returns `const int&` for read-only
  objects. This lets `Buffer` be used through const references/objects
  without losing read functionality.
- **Day 2 — Rule of 3/5/0**: if a class defines a custom destructor
  because it manages a resource manually, it normally must also define
  copy and move (Rule of 5), or delegate the resource to a type that
  already manages itself, such as `std::vector` or `std::unique_ptr`
  (Rule of 0). The copy constructor performs a **deep copy**: each
  object allocates its own block of memory, independent from the
  original. The copy assignment operator is implemented with
  **copy-and-swap** (builds a temporary via the copy constructor, then
  swaps it into `*this`), which also handles self-assignment correctly
  with no explicit check.
- **Day 3 — `std::move` / `std::forward`**: `std::move` is a cast to an
  rvalue reference that enables the move overload to be selected;
  `std::forward` is used in templates with forwarding references
  (`T&&`) to preserve whether the original argument was an lvalue or an
  rvalue. The move constructor and move assignment operator are
  implemented (they steal the pointer with `std::exchange`, leaving the
  source as `nullptr`), plus a minimal `std::forward` example via
  `forward_and_show`.
- **Day 4 — manual validation**: `demo_copy_duplicates_resource()`
  proves that copying produces two distinct memory addresses and that
  modifying the copy doesn't affect the original; `demo_copy_assignment()`
  proves the same for `operator=`; `demo_move_transfers_no_duplicate()`
  and `demo_move_assignment()` prove that moving keeps the SAME memory
  address in the destination and leaves the source empty (no new
  allocation).
- **Day 5 — self-review and commit**: compile with `-Wall -Wextra
  -Wpedantic` with no warnings, review the code, and commit the
  checkpoint.

## Files

- `include/Buffer.hpp` — the `Buffer` class (full Rule of 5, header-only,
  no `<iostream>` dependency unless `BUFFER_TRACE` is defined; copy
  assignment uses copy-and-swap).
- `examples/demo_semana2.cpp` — a `main()` with the demonstrations
  (const-correctness, forwarding, deep copy, copy assignment, move,
  move assignment), compiled with console traces on.
- `tests/` — real tests with assertions and a non-zero exit code on
  failure, wired into CTest (`buffer_construccion`, `buffer_copia`,
  `buffer_movimiento`).

## Build and run

From the repo root (see the [root README](../../README.md) for the full setup):

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure -R buffer_
./build/semana2-regla-de-cinco/dia1-5/demo_semana2
```

## Validate with sanitizers

```bash
cmake -S . -B build-asan -DENABLE_SANITIZERS=ON
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure -R buffer_
```

Confirms there's no "invalid free" (which would betray a shallow copy
instead of a deep one) and no other invalid accesses across copy, copy
assignment, move, move assignment, and self-assignment. To check for
leaks on macOS, where ASan doesn't detect them, run
`leaks --atExit -- ./build/semana2-regla-de-cinco/dia1-5/demo_semana2`
(see the [root README](../../README.md)).
