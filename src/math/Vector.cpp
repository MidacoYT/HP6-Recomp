#include "math/Vector.h"
#include <cmath>

namespace hp6::math {

// ReAgent candidate: reports/pipeline/candidates/005d1680.cpp
void normalize_xyzw(const float* input, float* output) {
    const float x = input[0];
    const float scale = 1.0f / std::sqrt(
        x * x + input[1] * input[1] + input[2] * input[2] + input[3] * input[3]);
    output[0] = x * scale;
    output[1] = input[1] * scale;
    output[2] = scale * input[2];
    output[3] = scale * input[3];
}

// ReAgent candidate: reports/pipeline/candidates/005d1770.cpp
long double dot_xyz(const float* first, const float* second) {
    return static_cast<long double>(second[0]) * static_cast<long double>(first[0])
         + static_cast<long double>(second[1]) * static_cast<long double>(first[1])
         + static_cast<long double>(first[2]) * static_cast<long double>(second[2]);
}

// ReAgent candidate: reports/pipeline/candidates/00404670.cpp
std::uint32_t different_xyz(const float* first, const float* second) {
    if (second[0] == first[0] && second[1] == first[1] &&
        second[2] == first[2]) return 0;
    return 1;
}

// ReAgent candidate: reports/pipeline/candidates/00405390.cpp
void subtract_xyzw(float* output, const float* first, const float* second) {
    const float a0 = first[0], a1 = first[1], a2 = first[2], a3 = first[3];
    const float b0 = second[0], b1 = second[1], b2 = second[2], b3 = second[3];
    output[0] = a0 - b0;
    output[1] = a1 - b1;
    output[2] = a2 - b2;
    output[3] = a3 - b3;
}

// ReAgent candidates rejected: reports/pipeline/rejected/005d16f0.cpp
// The x86 sequence computes: out.x = a.y*b.z - a.z*b.y, then continues for
// out.y and out.z. No fourth output is written; the signature is (a,b,out).
void cross_xyz(const float* a, const float* b, float* output) {
    output[0] = a[1] * b[2] - a[2] * b[1];
    output[1] = a[2] * b[0] - a[0] * b[2];
    output[2] = a[0] * b[1] - a[1] * b[0];
}

}
