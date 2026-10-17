#pragma once

#include <cstdint>

namespace hp6::math {

// The pointer layouts follow the observed float[4] accesses. Ownership and
// a C++ class type have not been established by the exports.
void normalize_xyz_preserve_w(const float* input, float* output); // 0x005d1580
void normalize_xyzw(const float* input, float* output);          // 0x005d1680
void cross_xyz(const float* a, const float* b, float* output);   // 0x005d16f0
long double dot_xyz(const float* first, const float* second);   // 0x005d1770
std::uint32_t different_xyz(const float* first, const float* second); // 0x00404670
void subtract_xyzw(float* output, const float* first, const float* second); // 0x00405390

}
