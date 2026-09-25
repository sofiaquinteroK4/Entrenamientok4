# Ownership and Smart Pointers (Week 1, Day 1-3)

## Weekly plan

- **Day 1 — ownership review**: why a stack object destroys itself
  automatically when it goes out of scope, and why the heap needs an
  explicit `delete` (and what happens if you forget it). `LifetimeTracker`
  keeps a static count of live instances so the difference can be
  observed with real data, not just theory. The repo was also set up
  (CMake + CTest + `.gitignore`, see the [root README](../../README.md))
  for the rest of the month.
- **Day 2 — `unique_ptr`**: ownership transfer (`unique_ptr` can't be
  copied, only moved; the source becomes `nullptr` after `std::move`)
  and custom deleters (`HandleDeleter` simulates releasing a resource
  from a C-style API that doesn't use `delete`).
- **Day 3 — `shared_ptr`/`weak_ptr`**: reference counting (`use_count()`),
  and the circular reference problem: `CyclicNodeBroken` shows how two
  `shared_ptr`s pointing at each other never reach 0 references (a
  leak), and `CyclicNodeFixed` solves it by replacing one side with a
  `weak_ptr` (which doesn't add to the count).

## Files

- `include/dia1/LifetimeTracker.hpp`, `include/dia2/HandleDeleter.hpp`,
  `include/dia3/CyclicNode.hpp` — the supporting class for each day,
  with no `<iostream>`.
- `examples/demo_dia1_ownership.cpp`, `demo_dia2_unique_ptr.cpp`,
  `demo_dia3_shared_weak.cpp` — one `main()` per day with the
  demonstrations and console traces.
- `tests/` — real tests with assertions and a non-zero exit code on
  failure, wired into CTest (`dia1_ownership`, `dia2_unique_ptr`,
  `dia3_shared_weak`).

## Build and run

From the repo root (see the [root README](../../README.md) for the full
setup):

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure -R dia[123]_
./build/semana1-raii-smartpointers/dia1-3/demo_dia1_ownership
./build/semana1-raii-smartpointers/dia1-3/demo_dia2_unique_ptr
./build/semana1-raii-smartpointers/dia1-3/demo_dia3_shared_weak
```

## Validate with sanitizers

```bash
cmake -S . -B build-asan -DENABLE_SANITIZERS=ON
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure -R dia[123]_
```

Note on `demo_dia3_shared_weak`: the "broken cycle" demo deliberately
creates a circular reference with `shared_ptr` to show the problem, but
breaks it by hand (`raw1->next.reset()`) before it ends, so no real leak
is left at exit. On macOS, where ASan does not detect leaks, confirm it
with `leaks --atExit -- ./build/semana1-raii-smartpointers/dia1-3/demo_dia3_shared_weak`
(see the [root README](../../README.md)).
