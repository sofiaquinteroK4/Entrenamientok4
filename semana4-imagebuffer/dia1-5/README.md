# ImageBuffer — Month 1 Consolidation (Week 4)

`ImageBuffer` is an image held in one contiguous heap block of 8-bit
values: `width x height` pixels with `channels` values each (1 =
grayscale, 3 = RGB, 4 = RGBA). It brings the whole month together:

| Week | Concept | Where it shows up in `ImageBuffer` |
| --- | --- | --- |
| 1 | RAII | The constructor allocates, the destructor frees; a rejected image never allocates |
| 2 | Rule of 5, deep copy, move, const-correctness | The five special members, written by hand; `const` overloads of `at()`/`data()`/`begin()`/`end()` |
| 3 | Idiomatic STL + unit tests + CTest | `begin()`/`end()`, and `std::copy`/`std::fill`/`std::equal` instead of loops; AAA tests registered with `add_test()` |
| Feedback | `[[nodiscard]]`, `noexcept`, no dependence on member order | Every getter; `data_` is computed from the constructor parameters |

## Day 1 — API design

```cpp
class ImageBuffer {
public:
    using value_type = std::uint8_t;          // one channel of one pixel
    using iterator = value_type*;
    using const_iterator = const value_type*;
    static constexpr std::size_t max_channels = 4;

    ImageBuffer() noexcept;                                    // empty 0 x 0 x 0
    ImageBuffer(std::size_t width, std::size_t height, std::size_t channels);
    ~ImageBuffer();
    ImageBuffer(const ImageBuffer&);                           // deep copy
    ImageBuffer& operator=(const ImageBuffer&);                // copy-and-swap
    ImageBuffer(ImageBuffer&&) noexcept;                       // steals the block
    ImageBuffer& operator=(ImageBuffer&&) noexcept;            // move-and-swap
    void swap(ImageBuffer&) noexcept;                          // + free swap()

    std::size_t width() const noexcept;
    std::size_t height() const noexcept;
    std::size_t channels() const noexcept;
    std::size_t size() const noexcept;       // width * height * channels (= bytes)
    bool empty() const noexcept;

    value_type* data() noexcept;             // + const overload
    iterator begin() noexcept;               // + const overloads, end()
    value_type& at(std::size_t x, std::size_t y, std::size_t channel);  // + const
    void fill(value_type value) noexcept;
};

bool operator==(const ImageBuffer&, const ImageBuffer&);  // + operator!=
```

### Design decisions

- **`std::uint8_t` per channel:** it's the format of most images (PNG,
  JPEG, what a camera hands out) and makes `size()` equal to the size
  in bytes.
- **Memory layout: row-major, interleaved.** Pixel `(x, y)`, channel
  `c` lives at index `(y * width + x) * channels + c` (`R G B R G B
  ...`, row after row). It's what stb_image, OpenCV, and PNG decoders
  use, so `data()` can be passed straight to them.
- **`at(x, y, channel)` in that order:** `x` is the column and `y` the
  row, as in image coordinates. It always checks bounds and throws
  `std::out_of_range`; there's no unchecked accessor, because the goal
  here is safety, not maximum speed.
- **Validation in the constructor, before allocating:**
  - `channels` outside `[1, 4]` → `std::invalid_argument`.
  - `width * height * channels` doesn't fit in a `std::size_t` →
    `std::length_error`. The check divides instead of multiplying, so
    the check itself can't overflow. Without it, a huge image would
    wrap around to a small allocation, and `at()` would then write past
    it.
  - `width == 0` or `height == 0` is valid: an empty image that keeps
    its `channels` and doesn't allocate.
- **A single "empty" state.** The default constructor and a moved-from
  object are both `0 x 0 x 0` with `data() == nullptr`. After a move,
  **all** dimensions go back to 0, not just the pointer: otherwise
  `width()`/`height()` would describe pixels that no longer exist.
- **Copy assignment with copy-and-swap** (strong guarantee: if the
  allocation fails, `*this` is left untouched) and **move assignment
  with move-and-swap** (the old block dies in the temporary). Neither
  needs an `if (this != &other)` check: self-assignment works on its
  own.
- **Moves are `noexcept`:** so `std::vector<ImageBuffer>` moves images
  when it reallocates instead of copying them (checked with
  `static_assert` in the tests).
- **`operator==` compares dimensions + pixels.** `size()` isn't
  enough: a 2 x 3 and a 3 x 2 image have the same number of values but
  aren't the same image.
- **Why not Rule of 0?** With a `std::vector<std::uint8_t>` member, the
  five special members would come for free and be correct. It's written
  by hand on purpose, to practice the Rule of 5 as the month's
  checkpoint. The moved-from state (`0 x 0 x 0`) would need care even
  with a `vector`, since the dimensions aren't reset automatically.

## Days 2-3 — Implementation

Everything is in `include/ImageBuffer.hpp` (header-only, with no
`<iostream>`). Days 2 and 3 are the constructors/destructor/accessors
and the copy/move members, respectively.

## Day 4 — Test suite

### What to cover and the test plan

Each test function covers one behavior and follows arrange-act-assert
(see week 3). Categories:

- **Normal case:** construction, access, copy, and move with real
  images.
- **Edge cases:** empty image (in all three forms), first/last pixel,
  `channels` at 1 and 4, self-assignment.
- **Errors:** invalid `channels`, size overflow, `at()` out of range on
  each coordinate.
- **Invariants:** a copy is independent, a move doesn't allocate, the
  moved-from object is empty and reusable, no block is leaked.

| CTest case | File | What it covers |
| --- | --- | --- |
| `imagebuffer_construccion` | `tests/test_construccion.cpp` | Dimensions and `size()`, zero-init, `channels` in `[1, 4]`, overflow rejected before allocating, memory layout, `at()` bounds on x/y/channel (mutable and const), `fill()` |
| `imagebuffer_vacio` | `tests/test_vacio.cpp` | **Empty buffer:** default, 0-wide or 0-tall; `begin() == end()`, `data() == nullptr`, `at()` throws, copying/moving an empty image, empty images compared by dimensions |
| `imagebuffer_copia` | `tests/test_copia.cpp` | **Independence after copy:** new block, same pixels; modifying the copy doesn't touch the original and vice versa; copy assignment with different dimensions (grows and shrinks), frees the old block; self-assignment |
| `imagebuffer_movimiento` | `tests/test_movimiento.cpp` | **State after move:** same block (no allocation), source `0 x 0 x 0` and reusable, move assignment frees the old block, self-move, `swap`, `noexcept` + `std::vector` reallocation |

### Verification with sanitizers and leak checkers

Three layers, each catching what the others can miss:

1. **ASan + UBSan** (`-DENABLE_SANITIZERS=ON`): use-after-free, double
   free, out-of-bounds, undefined behavior.
2. **Allocation counter** (`tests/alloc_counter.hpp`): replaces the
   global `operator new[]`/`delete[]` in the tests and counts live
   blocks. Each test ends with `CHECK(alloc_counter::live_arrays == 0)`,
   and the copy/move assignment tests check that the old block is freed
   **right away**. It's exact and works the same on macOS, Linux, and in
   the ASan build.
3. **valgrind / `leaks` as CTest tests** (label `memcheck`): each test
   executable and the demo are run again under valgrind if it's
   installed (Linux), or under macOS's `leaks --atExit` otherwise. They
   are skipped in the ASan build, because ASan replaces
   `malloc`/`free`.

Why the counter is needed: `leaks` and valgrind are *conservative*. A
stale copy of a pointer left on the stack makes a leaked block look
reachable. When checking the tests (below), a move assignment that
overwrote `data_` without freeing it slipped past both `leaks` and
ASan, and only the counter caught it.

### How we know the tests are useful

Each bug was injected into a copy of the header, and at least one
layer caught it:

| Bug injected | Caught by |
| --- | --- |
| Destructor doesn't `delete[]` | Allocation counter in all 4 tests + `*_memcheck` |
| Shallow copy (copies the pointer) | `imagebuffer_copia` (`copy.data() != original.data()`) + ASan (double free) |
| Move constructor doesn't reset `width_` | `imagebuffer_movimiento` (`source.width() == 0 ...`) |
| Move assignment overwrites without freeing | `imagebuffer_movimiento` (counter: `live_before - 1`) |
| `at()` with `>` instead of `>=` | `imagebuffer_construccion`, `imagebuffer_vacio` |
| No `channels` validation | `imagebuffer_construccion` |
| `data_` computed from members + declaration order swapped (the reviewer's week 2 case) | Doesn't compile (`-Wreorder-ctor`, `-Wuninitialized` with `-Werror`); without `-Werror`, `imagebuffer_construccion` aborts |

## Build and run

From the repo root (see the [root README](../../README.md) for the full
setup):

```bash
cmake -S . -B build
cmake --build build -j
ctest --test-dir build --output-on-failure -L semana4   # 4 cases + 5 memcheck
./build/semana4-imagebuffer/dia1-5/demo_imagebuffer
```

Sanitizers:

```bash
cmake -S . -B build-asan -DENABLE_SANITIZERS=ON
cmake --build build-asan -j
ctest --test-dir build-asan --output-on-failure -L semana4   # the 4 cases
```

Only the leak checks: `ctest --test-dir build -L memcheck`.

## Day 5 — Final self-review

- [x] Compiles with no warnings with `-Wall -Wextra -Wpedantic -Wshadow
      -Wconversion` (now the default for the whole repo) and with
      `-Werror`.
- [x] `ctest` green in the normal build (including memcheck) and with
      ASan + UBSan.
- [x] Constructor independent of member declaration order.
- [x] `[[nodiscard]]` on getters and `at()`; `noexcept` on everything
      that can't throw, and on moves.
- [x] No manual loops where an algorithm applies (`std::copy`,
      `std::fill`, `std::equal`). The ones left in the demo walk 2D
      coordinates to draw.
- [x] The demo doesn't claim "no leaks": it says how to verify it.
- [x] Every test was checked against an injected bug.

## Notes for the mentor's review

Points worth discussing:

- **Rule of 5 vs Rule of 0:** the manual version is on purpose (see
  "Design decisions"). Would a `std::vector` version be worth doing as
  a comparison?
- **`at()` always checks bounds.** For image processing loops, an
  unchecked `operator()(x, y, c)` (like `std::vector::operator[]`) would
  be the next step.
- **`alloc_counter.hpp` replaces `operator new[]` globally** in the test
  executables. It's allowed by the standard, but it only counts array
  allocations and must be included from exactly one `.cpp` per
  executable.
- **valgrind:** on this machine (macOS, Apple Silicon) there's no
  valgrind, so the `*_memcheck` tests use `leaks`. On Linux, the same
  `ctest -L memcheck` uses valgrind automatically.
- **`check.hpp`** is a copy of week 3's, so each week stays
  self-contained.
