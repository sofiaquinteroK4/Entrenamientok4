# C++ Training

Exercises from the training plan, organized by week. Starting from Month 1,
CMake + CTest is the only build and test stack: each week exposes a
header-only library in `include/`, a demo executable in `examples/` (with
the teaching prints), and real assertion-based tests in `tests/`, with no
external frameworks.

## Folder -> plan map

| Folder | Week / Days | Topic | Status |
| --- | --- | --- | --- |
| `semana1-raii-smartpointers/dia1-3/` | Week 1, Days 1-3 | Ownership (`LifetimeTracker`), `unique_ptr` (transfer + deleters), `shared_ptr`/`weak_ptr` (reference counting + circular reference) | Done |
| `semana1-raii-smartpointers/dia4-5/` | Week 1, Days 4-5 | RAII over a raw buffer (`RaiiBuffer`, move-only) | Done |
| `semana2-regla-de-cinco/dia1-5/` | Week 2, Days 1-5 | const-correctness + Rule of 3/5/0 + deep copy + move (`Buffer`) | Done |

## Build and test (whole repo)

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure
```

This compiles every week with `-Wall -Wextra -Wpedantic` and runs every
`add_test()` in the repo (one per case: construction, copy, move...).

### With sanitizers (ASan + UBSan)

```bash
cmake -S . -B build-asan -DENABLE_SANITIZERS=ON
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure
```

Replaces the manual `valgrind` usage from earlier weeks: runs on
macOS/Linux with no external dependencies and catches both leaks/invalid
accesses (ASan) and undefined behavior (UBSan).

### Running the demos (with console traces)

The `examples/` executables do print traces (`RAII_TRACE` / `BUFFER_TRACE`
where it applies), unlike the library and the tests:

```bash
./build/semana1-raii-smartpointers/dia1-3/demo_dia1_ownership
./build/semana1-raii-smartpointers/dia1-3/demo_dia2_unique_ptr
./build/semana1-raii-smartpointers/dia1-3/demo_dia3_shared_weak
./build/semana1-raii-smartpointers/dia4-5/demo_raii_buffer
./build/semana2-regla-de-cinco/dia1-5/demo_semana2
```

## Note on the environment

If `cmake` fails to detect the compiler with a linker error about unknown
architectures in an SDK `.tbd` file (Command Line Tools out of sync with
the default SDK on macOS), pin a more stable SDK explicitly:

```bash
cmake -S . -B build -DCMAKE_OSX_SYSROOT="$(ls -d /Library/Developer/CommandLineTools/SDKs/MacOSX26*.sdk | sort -V | tail -1)"
```
