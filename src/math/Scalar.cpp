#include "math/Scalar.h"
#include <algorithm>
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

// 0x00401270 (part of the float_ops_015 batch; ReAgent was unavailable
// for it, so this is a manual reconstruction from live disassembly, not
// the decompiled C alone -- Ghidra types the function `undefined`
// because every path ends in a bare `return;`, with the real float
// result left in XMM0 at that point, invisible to the decompiled C).
//
// A 16th-order even-power Taylor polynomial approximation of cos(x).
// All 9 coefficients (1, -1/2!, 1/4!, -1/6!, ..., 1/16!) and the pi/2pi/
// (1/2pi) constants are read directly from the binary (0x0087e990..ac,
// 0x00866e70, 0x0087ea90/94/98) -- confirmed to match cos's real Taylor
// series exactly, not recomputed from a library constant.
//
// Range reduction (traced instruction-by-instruction, since the
// decompiled C's "if (cond) return; return;" shape hides which branch
// carries the real result): fold the angle into one period, then twice
// more by reflection (first around pi, matching cos(x) == cos(2pi - x);
// then around pi/2) to land in [0, pi/2] for the polynomial, which only
// needs to be accurate near zero. The second reflection changes the
// sign of the true cosine value (cos(pi - r) == -cos(r)), so a flip
// flag tracks whether that branch was taken -- computed as an exact
// `A - A == 0` floating-point identity (robust regardless of rounding
// in `pi - r1` itself, since it is reused rather than recomputed).
float fast_cos(float angle) {
    constexpr float kInvTwoPi = 0.15915493667125702f;
    constexpr float kTwoPi = 6.2831854820251465f;
    constexpr float kPi = 3.1415927410125732f;
    constexpr float c0 = 1.0f;
    constexpr float c1 = -0.5f;
    constexpr float c2 = 0.0416666679084301f;
    constexpr float c3 = -0.0013888889225199819f;
    constexpr float c4 = 2.4801587642286904e-05f;
    constexpr float c5 = -2.755731998149713e-07f;
    constexpr float c6 = 2.0876755879584152e-09f;
    constexpr float c7 = -1.1470746227870698e-11f;
    constexpr float c8 = 4.7794772561329454e-14f;

    const float r0 = angle - static_cast<float>(static_cast<int>(angle * kInvTwoPi)) * kTwoPi;
    const float two_pi_minus_r0 = kTwoPi - r0;
    const float r1 = std::min(r0, two_pi_minus_r0);
    const float pi_minus_r1 = kPi - r1;
    const float r2 = std::min(r1, pi_minus_r1);
    const bool needs_flip = (pi_minus_r1 - r2) == 0.0f;

    const float x2 = r2 * r2;
    const float poly = c0 + x2 * (c1 + x2 * (c2 + x2 * (c3 + x2 * (c4 + x2 * (c5 + x2 *
                        (c6 + x2 * (c7 + x2 * c8)))))));

    return needs_flip ? -poly : poly;
}

}

