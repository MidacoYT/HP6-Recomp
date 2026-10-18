#pragma once
#include <cstdint>

namespace hp6::math {

// Ghidra float10 return. MSVC long double is only 64 bits; see README.
long double sqrt_x87(float value); // 0x00401020
long double abs_x87(float value);  // 0x00404660
std::uint32_t is_any_nan_xyz(const float* v);     // 0x005e8b50
std::uint32_t slope_greater(const float* a, const float* b); // 0x0068c0a0
long double crt_math_wrapper(float value);        // 0x00401000, appelle global 0x006f519c (CRT)
std::uint32_t range_check(float a, float b);      // 0x00401da0, a > |b - a|
float approx_transcendental_1(float x);           // 0x00401270, polynome avec globales
float approx_transcendental_2(float x);           // 0x004013a0, utilise sqrt_x87 + global 0x007db234
float three_param_float(float a, float b, float c); // 0x00403190, utilise global 0x007db234

}
