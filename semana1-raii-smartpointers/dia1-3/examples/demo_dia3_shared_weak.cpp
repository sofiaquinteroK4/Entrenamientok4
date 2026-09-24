#include "dia3/CyclicNode.hpp"

#include <iostream>
#include <memory>

// Day 3: shared_ptr (reference counting) and weak_ptr (breaks cycles).

void demo_reference_counting() {
    std::cout << "\n--- Day 3: shared_ptr counts references ---\n";

    auto a = std::make_shared<int>(100);
    std::cout << "  use_count() after creating 'a' = " << a.use_count() << "\n";

    auto b = a; // copy: 'a' and 'b' share the same resource
    std::cout << "  use_count() after copying into 'b' = " << a.use_count() << "\n";

    b.reset(); // 'b' drops its reference
    std::cout << "  use_count() after b.reset() = " << a.use_count() << "\n";
}

void demo_broken_cycle() {
    std::cout << "\n--- Day 3: circular reference with shared_ptr (leak) ---\n";

    // We keep a raw pointer (it doesn't add to the count) only so we
    // can break the cycle by hand after demonstrating the leak; in a
    // real program, without that external access, this block of
    // memory would be lost forever.
    CyclicNodeBroken* raw1 = nullptr;
    {
        auto node1 = std::make_shared<CyclicNodeBroken>();
        auto node2 = std::make_shared<CyclicNodeBroken>();
        node1->next = node2;
        node2->next = node1; // cycle: node1 <-> node2
        raw1 = node1.get();

        std::cout << "  use_count(node1) = " << node1.use_count()
                  << ", use_count(node2) = " << node2.use_count()
                  << " (each has 2 owners: the local variable and the "
                     "other one's 'next')\n";
    } // 'node1' and 'node2' (the local variables) go out of scope here...

    std::cout << "  After leaving the scope: alive_count() = "
              << CyclicNodeBroken::alive_count
              << " (this should be 0! the shared_ptr<->shared_ptr cycle "
                 "keeps them alive -> a real memory leak)\n";

    std::cout << "  Breaking the cycle by hand (raw1->next.reset()) so "
                 "this leak doesn't actually keep running...\n";
    raw1->next.reset();
    std::cout << "  alive_count() after breaking the cycle = "
              << CyclicNodeBroken::alive_count << "\n";
}

void demo_fixed_cycle() {
    std::cout << "\n--- Day 3: weak_ptr breaks the cycle ---\n";

    std::weak_ptr<CyclicNodeFixed> watcher;

    {
        auto node1 = std::make_shared<CyclicNodeFixed>();
        auto node2 = std::make_shared<CyclicNodeFixed>();
        node1->next = node2; // node1 "owns" node2 (shared_ptr)
        node2->prev = node1; // node2 only "observes" node1 (weak_ptr)
        watcher = node1;

        std::cout << "  use_count(node1) = " << node1.use_count()
                  << " (node2->prev's weak_ptr does NOT add to the count)\n";
    } // both are destroyed here: with no shared_ptr cycle, no leak.

    std::cout << "  After leaving the scope: alive_count() = "
              << CyclicNodeFixed::alive_count << " (0: no leak)\n";

    if (watcher.expired()) {
        std::cout << "  watcher.expired() = true: the weak_ptr did not "
                     "keep the object alive, and lock() now returns "
                     "nullptr\n";
    }
}

int main() {
    demo_reference_counting();
    demo_broken_cycle();
    demo_fixed_cycle();
    std::cout << "\nProgram finished with no memory leaks.\n";
    return 0;
}
