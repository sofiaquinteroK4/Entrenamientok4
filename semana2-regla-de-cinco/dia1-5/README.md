# Rule of 3/5/0 + Deep Copy + Move (Week 2, Day 1-5)

Follow-up to the Week 1 exercise (`RaiiBuffer`, which only allowed
moving). Here `Buffer` implements the full Rule of 5: besides moving, it
can now also be **copied**, performing a **deep copy** of the heap
resource.

## Weekly plan

- **Day 1 — const-correctness**: methods that don't modify the object
  are marked `const` (`size()`, `direccion()`, `imprimir()`), and `at()`
  has a `const` overload that returns `const int&` for read-only
  objects. This lets `Buffer` be used through const references/objects
  without losing read functionality.
- **Day 2 — Rule of 3/5/0**: if a class defines a custom destructor
  because it manages a resource manually, it normally must also define
  copy and move (Rule of 5), or delegate the resource to a type that
  already manages itself, such as `std::vector` or `std::unique_ptr`
  (Rule of 0). The copy constructor and copy assignment operator are
  implemented with a **deep copy**: each object allocates its own block
  of memory, independent from the original.
- **Day 3 — `std::move` / `std::forward`**: `std::move` is a cast to an
  rvalue reference that enables the move overload to be selected;
  `std::forward` is used in templates with forwarding references
  (`T&&`) to preserve whether the original argument was an lvalue or an
  rvalue. The move constructor and move assignment operator are
  implemented (they steal the pointer with `std::exchange`, leaving the
  source as `nullptr`), plus a minimal `std::forward` example via
  `reenviar_y_mostrar`.
- **Day 4 — manual validation**: `demo_copia_duplica_recurso()` proves
  that copying produces two distinct memory addresses and that
  modifying the copy doesn't affect the original; `demo_mover_no_duplica()`
  proves that moving keeps the SAME memory address in the destination
  and leaves the source empty (no new allocation).
- **Day 5 — self-review and commit**: compile with `-Wall -Wextra
  -Wpedantic` with no warnings, review the code, and commit the
  checkpoint.

## Files

- `buffer_rule_of_five.cpp` — the `Buffer` class (full Rule of 5) plus a
  `main()` with the four demonstrations (const-correctness, forwarding,
  deep copy, move).

## Build and run

```bash
g++ -std=c++17 -Wall -Wextra -Wpedantic -o buffer_rule_of_five buffer_rule_of_five.cpp
./buffer_rule_of_five
```

## Validate with Valgrind (Linux)

```bash
g++ -std=c++17 -g -o buffer_rule_of_five buffer_rule_of_five.cpp
valgrind --leak-check=full ./buffer_rule_of_five
```

Confirm the output ends with `All heap blocks were freed -- no leaks
are possible` and that no "invalid free" shows up (which would betray a
shallow copy instead of a deep one).
