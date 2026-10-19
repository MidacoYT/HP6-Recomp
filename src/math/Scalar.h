#pragma once
#include <cstdint>

namespace hp6::math {

// Ghidra float10 return. MSVC long double is only 64 bits; see README.
long double sqrt_x87(float value); // 0x00401020
long double abs_x87(float value);  // 0x00404660
std::uint32_t any_nonzero_xyz(const float* v);     // 0x005e8b50
std::uint32_t slope_greater(const float* a, const float* b); // 0x0068c0a0

// A custom cosine approximation (16th-order even polynomial, quadrant
// range reduction) -- see Scalar.cpp for the full derivation from the
// real x87/SSE disassembly.
float fast_cos(float angle); // 0x00401270

}
