#include "dia3/CyclicNode.hpp"

#include <iostream>
#include <memory>

int main() {
    // --- Basic reference counting ---
    auto a = std::make_shared<int>(1);
    if (a.use_count() != 1) {
        std::cerr << "FAIL: expected initial use_count() 1\n";
        return 1;
    }
    auto b = a;
    if (a.use_count() != 2) {
        std::cerr << "FAIL: expected use_count() 2 after copying\n";
        return 1;
    }
    b.reset();
    if (a.use_count() != 1) {
        std::cerr << "FAIL: expected use_count() 1 after reset()\n";
        return 1;
    }

    // --- Broken cycle: demonstrates the leak and cleans it up by hand ---
    if (CyclicNodeBroken::alive_count != 0) {
        std::cerr << "FAIL: CyclicNodeBroken::alive_count should start at 0\n";
        return 1;
    }
    CyclicNodeBroken* raw1 = nullptr;
    {
        auto node1 = std::make_shared<CyclicNodeBroken>();
        auto node2 = std::make_shared<CyclicNodeBroken>();
        node1->next = node2;
        node2->next = node1;
        raw1 = node1.get();
    }
    if (CyclicNodeBroken::alive_count != 2) {
        std::cerr << "FAIL: a shared_ptr cycle should keep both nodes "
                     "alive after leaving scope (leak expected)\n";
        return 1;
    }
    raw1->next.reset(); // break the cycle by hand so the leak doesn't keep running
    if (CyclicNodeBroken::alive_count != 0) {
        std::cerr << "FAIL: breaking the cycle by hand should free both nodes\n";
        return 1;
    }

    // --- Fixed cycle using weak_ptr ---
    if (CyclicNodeFixed::alive_count != 0) {
        std::cerr << "FAIL: CyclicNodeFixed::alive_count should start at 0\n";
        return 1;
    }
    std::weak_ptr<CyclicNodeFixed> watcher;
    {
        auto node1 = std::make_shared<CyclicNodeFixed>();
        auto node2 = std::make_shared<CyclicNodeFixed>();
        node1->next = node2;
        node2->prev = node1;
        watcher = node1;

        if (node1.use_count() != 1) {
            std::cerr << "FAIL: node2->prev's weak_ptr should not add "
                         "to node1's use_count()\n";
            return 1;
        }
    }
    if (CyclicNodeFixed::alive_count != 0) {
        std::cerr << "FAIL: with no shared_ptr cycle, both nodes should "
                     "destroy themselves when leaving scope\n";
        return 1;
    }
    if (!watcher.expired()) {
        std::cerr << "FAIL: watcher.expired() should be true after the "
                     "object is destroyed\n";
        return 1;
    }
    if (watcher.lock() != nullptr) {
        std::cerr << "FAIL: lock() on an expired weak_ptr should return "
                     "nullptr\n";
        return 1;
    }

    std::cout << "OK: shared_ptr counts references correctly; a "
                 "shared_ptr<->shared_ptr cycle leaks until broken by "
                 "hand; with weak_ptr the cycle doesn't leak and expires "
                 "on its own\n";
    return 0;
}
