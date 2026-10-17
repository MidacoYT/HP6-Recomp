#include "math/Scalar.h"
#include <cmath>

namespace hp6::math {

// ReAgent candidate: reports/pipeline/candidates/00401020.cpp
long double sqrt_x87(float value) {
    return std::sqrt(static_cast<long double>(value));
}

// ReAgent candidate: reports/pipeline/candidates/00404660.cpp
// The candidate requires x87 long double. The modern API retains the
// operation while ABI equivalence remains unverified on MSVC.
long double abs_x87(float value) {
    return std::fabs(static_cast<long double>(value));
}

// ReAgent candidate rejected: reports/pipeline/rejected/005e8b50.cpp
// UCOMISS + LAHF + TEST AH,0x44 + JP returns 1 for either ordered
// nonzero comparison or unordered comparison, and 0 only for equality to zero.
std::uint32_t any_nonzero_xyz(const float* v) {
    for (int i = 0; i < 3; ++i) {
        if (v[i] != 0.0f) {
            return 1u;
        }
    }
    return 0u;
}

// ReAgent candidate rejected: reports/pipeline/rejected/0068c0a0.cpp
// x87 FCOMPP compares a[1]*b[0] with b[1]*a[0]. TEST AH,0x41 checks C0|C3.
// Returns 1 only if a[1]*b[0] > b[1]*a[0] (strictly greater, ordered).
// Returns 0 for equal, less, or unordered (NaN).
std::uint32_t slope_greater(const float* a, const float* b) {
    const float left = a[1] * b[0];
    const float right = b[1] * a[0];
    // Ordered comparison: NaN comparisons are false, so this yields 0 for NaN.
    return (left > right) ? 1u : 0u;
}


}

