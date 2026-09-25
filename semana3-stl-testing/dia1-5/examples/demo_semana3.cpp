#include "Buffer.hpp"

#include <algorithm>
#include <functional> // std::greater
#include <iostream>
#include <string>     // std::to_string
#include <numeric>    // std::iota, std::accumulate

// ---------------------------------------------------------------------
// Manual loop vs <algorithm>, side by side.
//
// Each section solves the same problem twice: first with the manual
// loop you'd write by hand, then with the standard algorithm that
// already does it. Both results are printed so you can see they match.
// The algorithm version says WHAT it does in its name (count_if,
// find_if, all_of...), while the loop makes the reader reverse-engineer
// the intent from indexes and flags.
// ---------------------------------------------------------------------

void print(const char* label, const Buffer& buffer) {
    std::cout << "  " << label << ": [";
    bool first = true;
    for (int value : buffer) { // range-for works thanks to begin()/end()
        std::cout << (first ? "" : ", ") << value;
        first = false;
    }
    std::cout << "]\n";
}

void demo_iota() {
    std::cout << "\n--- Fill with 0, 1, 2, ... ---\n";
    Buffer manual(5);
    for (std::size_t i = 0; i < manual.size(); ++i) {
        manual.at(i) = static_cast<int>(i);
    }
    print("manual loop    ", manual);

    Buffer idiomatic(5);
    std::iota(idiomatic.begin(), idiomatic.end(), 0);
    print("std::iota      ", idiomatic);
}

void demo_accumulate(const Buffer& numbers) {
    std::cout << "\n--- Sum all elements ---\n";
    int manual = 0;
    for (std::size_t i = 0; i < numbers.size(); ++i) {
        manual += numbers.at(i);
    }
    std::cout << "  manual loop    : " << manual << "\n";

    int idiomatic = std::accumulate(numbers.begin(), numbers.end(), 0);
    std::cout << "  std::accumulate: " << idiomatic << "\n";
}

void demo_max_element(const Buffer& numbers) {
    std::cout << "\n--- Largest element ---\n";
    int manual = numbers.at(0);
    for (std::size_t i = 1; i < numbers.size(); ++i) {
        if (numbers.at(i) > manual) {
            manual = numbers.at(i);
        }
    }
    std::cout << "  manual loop     : " << manual << "\n";

    // Returns an iterator, not a value: on an empty range it returns
    // end() instead of reading out of bounds like the loop above would.
    auto it = std::max_element(numbers.begin(), numbers.end());
    std::cout << "  std::max_element: " << *it << "\n";
}

void demo_count_if(const Buffer& numbers) {
    std::cout << "\n--- How many are even ---\n";
    int manual = 0;
    for (std::size_t i = 0; i < numbers.size(); ++i) {
        if (numbers.at(i) % 2 == 0) {
            ++manual;
        }
    }
    std::cout << "  manual loop  : " << manual << "\n";

    auto idiomatic = std::count_if(numbers.begin(), numbers.end(),
                                   [](int n) { return n % 2 == 0; });
    std::cout << "  std::count_if: " << idiomatic << "\n";
}

void demo_find_if(const Buffer& numbers) {
    std::cout << "\n--- First element greater than 40 ---\n";
    bool found = false;
    std::size_t manual_index = 0;
    for (std::size_t i = 0; i < numbers.size(); ++i) {
        if (numbers.at(i) > 40) {
            found = true;
            manual_index = i;
            break;
        }
    }
    std::cout << "  manual loop : "
              << (found ? "index " + std::to_string(manual_index) : "not found")
              << "\n";

    // "Not found" is expressed as end(), no extra flag needed.
    auto it = std::find_if(numbers.begin(), numbers.end(),
                           [](int n) { return n > 40; });
    std::cout << "  std::find_if: "
              << (it != numbers.end()
                      ? "index " + std::to_string(it - numbers.begin())
                      : "not found")
              << "\n";
}

void demo_all_of(const Buffer& numbers) {
    std::cout << "\n--- Are all of them positive? ---\n";
    bool manual = true;
    for (std::size_t i = 0; i < numbers.size(); ++i) {
        if (numbers.at(i) <= 0) {
            manual = false;
            break;
        }
    }
    std::cout << "  manual loop: " << std::boolalpha << manual << "\n";

    bool idiomatic = std::all_of(numbers.begin(), numbers.end(),
                                 [](int n) { return n > 0; });
    std::cout << "  std::all_of: " << idiomatic << "\n";
}

void demo_transform(const Buffer& numbers) {
    std::cout << "\n--- Double every element into a new buffer ---\n";
    Buffer manual(numbers.size());
    for (std::size_t i = 0; i < numbers.size(); ++i) {
        manual.at(i) = numbers.at(i) * 2;
    }
    print("manual loop   ", manual);

    Buffer idiomatic(numbers.size());
    std::transform(numbers.begin(), numbers.end(), idiomatic.begin(),
                   [](int n) { return n * 2; });
    print("std::transform", idiomatic);
}

void demo_sort(Buffer numbers) {
    std::cout << "\n--- Sort (no manual equivalent worth writing) ---\n";
    std::sort(numbers.begin(), numbers.end());
    print("ascending ", numbers);
    std::sort(numbers.begin(), numbers.end(), std::greater<>());
    print("descending", numbers);
}

int main() {
    demo_iota();

    Buffer numbers(6);
    int values[] = {12, 7, 45, 3, 28, 50};
    std::copy(std::begin(values), std::end(values), numbers.begin());
    std::cout << "\nThe remaining demos use:\n";
    print("numbers", numbers);

    demo_accumulate(numbers);
    demo_max_element(numbers);
    demo_count_if(numbers);
    demo_find_if(numbers);
    demo_all_of(numbers);
    demo_transform(numbers);
    demo_sort(numbers); // takes a copy: 'numbers' stays unsorted

    std::cout << "\nProgram finished with no memory leaks.\n";
    return 0;
}
