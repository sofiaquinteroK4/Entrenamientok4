# Idiomatic STL + Testing Fundamentals + CTest (Week 3)

Follow-up to the Week 2 `Buffer` (full Rule of 5). This week it gains
`begin()`/`end()` so the standard algorithms can work on it, and the
tests move from "one `if` + `return 1` per check" to a small `CHECK`
helper with one function per behavior.

## Weekly plan

### 1. Replace manual loops with `<algorithm>`

`Buffer` exposes `begin()`/`end()` (raw pointers already are
random-access iterators), plus the `value_type`/`iterator`/
`const_iterator` aliases the standard containers have. From there:

| Manual loop | Algorithm | Where |
| --- | --- | --- |
| `for (i...) copy[i] = other[i]` | `std::copy` | copy constructor |
| `for (i...) data[i] = value` | `std::fill` | `Buffer::fill()` |
| size check + `for (i...) if (a[i] != b[i])` | `std::equal` (4 iterators) | `operator==` |
| `for (i...) if (at(i) != 0) fail` | `std::all_of` | `test_algoritmos.cpp` (was a loop in week 2's `test_construccion.cpp`) |
| accumulator + loop | `std::accumulate` | demo + tests |
| "best so far" + loop | `std::max_element` | demo + tests |
| counter + `if` | `std::count_if` | demo + tests |
| `found` flag + index + `break` | `std::find_if` (returns `end()` if not found) | demo + tests |
| `ok = true` + `break` | `std::all_of` | demo + tests |
| `out[i] = f(in[i])` | `std::transform` | demo + tests |
| `data[i] = i` | `std::iota` | demo + tests |

`examples/demo_semana3.cpp` shows each pair side by side (manual loop
first, then the algorithm) and prints both results so you can see they
match.

**Why:** the algorithm's name says what the code does (`count_if`,
`find_if`), with no indexes to get wrong by one, and edge cases come
built in: `max_element` on an empty range returns `end()`, while the
manual loop reads `at(0)` and throws.

**When a loop is still fine:** when the body doesn't match any single
algorithm, or it does several things at once (like `print()` in the
demo, which also handles the separator). Forcing an algorithm with a
convoluted lambda is worse than a clear range-for. Only week 2's
`test_construccion.cpp` had a manual loop; it is left as is because the
week 2 `Buffer` has no iterators (that folder is the week 2 checkpoint).

### 2. What a unit test is and why it matters

A **unit test** is a small program that runs ONE piece of code (here, a
`Buffer` behavior) in isolation and checks the result automatically:
it passes or fails, with no one reading the output.

- **Living documentation:** `copying_an_empty_buffer()` or
  `max_element_on_empty_returns_end()` say how the class behaves in the
  edge cases, and unlike a comment they can't go stale: if the behavior
  changes, the test fails.
- **Catching regressions early:** a regression is something that used
  to work and broke because of a later change. `test_regresion.cpp`
  pins down behaviors that are easy to break while refactoring
  (self-assignment, `noexcept` on the move operations, copying an empty
  buffer). The failure shows up when you run `ctest`, right after the
  change, not weeks later in some other part of the program.

How the tests are written:

- **One function = one behavior**, with a name that describes it
  (`find_if_found_and_not_found`), so the failure message already says
  what broke.
- **Arrange-act-assert:** set up the data, run the operation, check
  the result, separated by a blank line.
- **`tests/check.hpp`** (still no external framework): `CHECK(cond)`
  prints file, line, and the failed expression and **keeps going**, so
  one run shows every failure. `CHECK_THROWS(expr, Exception)` checks
  that something throws. `check::report()` turns the result into the
  exit code: 0 = passed, 1 = failed.
- **Checks at compile time** (`static_assert`) when the property is
  about types: const iterators, `noexcept` move operations.

**How to know a test is useful:** break the code on purpose and check
that a test fails. It was done for this week with a copy of the header:

| Bug injected into `Buffer.hpp` | Caught by |
| --- | --- |
| Remove `if (this != &other)` from move assignment | `stl_regresion` (ASan: heap-use-after-free on self-move) |
| `operator==` with 3-iterator `std::equal` (ignores size) | `stl_algoritmos`, line with `make_buffer({1, 2}) != make_buffer({1, 2, 3})` |
| Remove `noexcept` from the move constructor | `stl_regresion` doesn't even compile (`static_assert`) |

### 3. CMake + `enable_testing()` + `add_test()`

The pieces, from top to bottom:

1. The root `CMakeLists.txt` calls `enable_testing()` once. Without
   it, `add_test()` is ignored and `ctest` finds no tests.
2. Each week does `add_subdirectory(...)`, and this week's
   `CMakeLists.txt` defines `stl_buffer_lib` as an `INTERFACE` library
   (header-only: it only exports the `include/` path).
3. `tests/CMakeLists.txt` creates one executable per file and registers
   it with `add_test(NAME stl_<name> COMMAND test_stl_<name>)`, all in
   a `foreach` so each new test file only needs its name added to the
   list. Each test gets the `semana3` label.
4. `ctest` runs each command and marks it `Passed` if the exit code is
   0 and `Failed` otherwise. That's why `check::report()` returns 0 or 1.

Useful `ctest` commands:

```bash
ctest --test-dir build -N                        # list tests without running them
ctest --test-dir build -L semana3                # only this week's tests (by label)
ctest --test-dir build -R stl_regresion          # by name (regex)
ctest --test-dir build --output-on-failure       # show the output of failing tests
ctest --test-dir build --rerun-failed            # rerun only the ones that failed
```

## Files

- `include/Buffer.hpp` — `Buffer` with iterators, `fill()`, `empty()`,
  `data()`, `operator==`/`!=` and a free `swap`; special members use
  `<algorithm>`.
- `examples/demo_semana3.cpp` — manual loop vs algorithm, side by side.
- `tests/check.hpp` — `CHECK`, `CHECK_THROWS`, `check::report()`.
- `tests/test_iteradores.cpp` — `begin()`/`end()`, range-for, const
  iterators, bounds checks.
- `tests/test_algoritmos.cpp` — algorithms on a `Buffer`, including
  empty ranges and "not found".
- `tests/test_regresion.cpp` — self-assignment, copying empty buffers,
  `swap`, `noexcept` moves in `std::vector`.

## Build and run

From the repo root (see the [root README](../../README.md) for the full
setup):

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure -L semana3
./build/semana3-stl-testing/dia1-5/demo_semana3
```

## Validate with sanitizers and leaks

```bash
cmake -S . -B build-asan -DENABLE_SANITIZERS=ON
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure -L semana3
leaks --atExit -- ./build/semana3-stl-testing/dia1-5/demo_semana3 2>/dev/null | grep "leaks for"
```
