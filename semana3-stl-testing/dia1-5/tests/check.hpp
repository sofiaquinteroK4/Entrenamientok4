#pragma once

// Minimal test helper, still with no external framework.
//
// Weeks 1-2 wrote every check as `if (...) { cerr << ...; return 1; }`,
// which stops at the first failure and repeats the same four lines per
// check. CHECK records the failure (file, line, and the expression that
// failed) and keeps going, so a single run shows every broken behavior.
// check::report() turns the result into the exit code CTest looks at:
// 0 = passed, anything else = failed.

#include <iostream>

namespace check {
inline int failures = 0;

inline int report(const char* suite) {
    if (failures == 0) {
        std::cout << "OK: " << suite << "\n";
        return 0;
    }
    std::cerr << suite << ": " << failures << " check(s) failed\n";
    return 1;
}
} // namespace check

#define CHECK(condition)                                                  \
    do {                                                                  \
        if (!(condition)) {                                               \
            std::cerr << __FILE__ << ":" << __LINE__                      \
                      << ": FAIL: " #condition "\n";                      \
            ++check::failures;                                            \
        }                                                                 \
    } while (0)

// Passes only if `expression` throws an exception of type `exception`.
#define CHECK_THROWS(expression, exception)                               \
    do {                                                                  \
        bool threw_ = false;                                              \
        try {                                                             \
            (void)(expression);                                           \
        } catch (const exception&) {                                      \
            threw_ = true;                                                \
        }                                                                 \
        if (!threw_) {                                                    \
            std::cerr << __FILE__ << ":" << __LINE__                      \
                      << ": FAIL: " #expression " did not throw "         \
                         #exception "\n";                                 \
            ++check::failures;                                            \
        }                                                                 \
    } while (0)
