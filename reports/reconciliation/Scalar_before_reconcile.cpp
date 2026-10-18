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
// UCOMISS + LAHF + TEST AH,0x44 + JP detects unordered (NaN) per component.
// Returns 1 if any component is NaN, 0 otherwise.
std::uint32_t is_any_nan_xyz(const float* v) {
    for (int i = 0; i < 3; ++i) {
        const float x = v[i];
        if (!(x == x)) { // NaN check: x != x is true only for NaN
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

// Globales externes lues depuis le binaire Ghidra (valeurs inconnues).
extern "C" {
    extern double (*g_crt_math_006f519c)(double);   // 0x006f519c, appelle depuis 00401000
    extern float g_float_007db234;                   // 0x007db234, utilise par plusieurs
    extern float g_float_008482b4;                   // 0x008482b4
    extern float g_float_0087ea90;
    extern float g_float_0087ea98;
    extern float g_float_0087ea94;
    extern float g_float_0087e990;
    extern float g_float_0087e994;
    extern float g_float_0087e998;
    extern float g_float_0087e99c;
    extern float g_float_0087e9a0;
    extern float g_float_0087e9a4;
    extern float g_float_0087e9a8;
    extern float g_float_0087e9ac;
    extern float g_float_00831c2c;
    extern float g_float_0080a8d0;
    extern float g_float_00848388;
    extern float g_float_00805994;
    extern float g_float_00848384;
    extern float g_float_00848380;
    extern float g_float_0084837c;
    extern float g_float_00805998;

// Définitions des globales externes (valeurs par défaut, a corriger depuis le binaire).
double (*g_crt_math_006f519c)(double) = nullptr;
float g_float_007db234 = 0.0f;
float g_float_008482b4 = 0.0f;
float g_float_0087ea90 = 0.0f;
float g_float_0087ea98 = 0.0f;
float g_float_0087ea94 = 0.0f;
float g_float_0087e990 = 0.0f;
float g_float_0087e994 = 0.0f;
float g_float_0087e998 = 0.0f;
float g_float_0087e99c = 0.0f;
float g_float_0087e9a0 = 0.0f;
float g_float_0087e9a4 = 0.0f;
float g_float_0087e9a8 = 0.0f;
float g_float_0087e9ac = 0.0f;
float g_float_00831c2c = 0.0f;
float g_float_0080a8d0 = 0.0f;
float g_float_00848388 = 0.0f;
float g_float_00805994 = 0.0f;
float g_float_00848384 = 0.0f;
float g_float_00848380 = 0.0f;
float g_float_0084837c = 0.0f;
float g_float_00805998 = 0.0f;
}

// ReAgent candidate: reports/pipeline/candidates/00401000.cpp
// Appelle le global 0x006f519c (fonction CRT, probablement sqrt) avec la
// valeur convertie en double, retourne le resultat en float10 (long double).
long double crt_math_wrapper(float value) {
    if (!g_crt_math_006f519c) return 0.0L;
    return static_cast<long double>(g_crt_math_006f519c(static_cast<double>(value)));
}

// ReAgent candidate rejected: reports/pipeline/rejected/00401da0.cpp
// Calcule diff = b - a. Si diff < 0, result = g_008482b4 - diff, sinon result = diff.
// Retourne 1 si a > result, sinon 0.
std::uint32_t range_check(float a, float b) {
    float diff = b - a;
    if (diff < 0.0f) {
        diff = g_float_008482b4 - diff;
    }
    return (a > diff) ? 1u : 0u;
}

// ReAgent candidate: reports/pipeline/candidates/00401270.cpp
// Approximation polynomiale avec reduction d'argument. Utilise plusieurs
// globales comme coefficients. Les valeurs des globales sont inconnues,
// donc cette fonction est un placeholder structurel.
float approx_transcendental_1(float x) {
    (void)x;
    (void)g_float_0087ea90;
    (void)g_float_0087ea98;
    (void)g_float_0087ea94;
    (void)g_float_0087e990;
    (void)g_float_0087e994;
    (void)g_float_0087e998;
    (void)g_float_0087e99c;
    (void)g_float_0087e9a0;
    (void)g_float_0087e9a4;
    (void)g_float_0087e9a8;
    (void)g_float_0087e9ac;
    (void)g_float_00831c2c;
    return 0.0f;
}

// ReAgent candidate rejected: reports/pipeline/rejected/004013a0.cpp
// Utilise sqrt_x87 (0x00401020) et le global 0x007db234.
// Logique complexe avec branches conditionnelles sur NaN.
float approx_transcendental_2(float x) {
    (void)g_float_007db234;
    (void)g_float_008482b4;
    if (!(x == x)) { // NaN
        return 0.0f;
    }
    return x;
}

// ReAgent candidate: reports/pipeline/candidates/00403190.cpp
// Utilise le global 0x007db234 et fait des divisions conditionnelles.
float three_param_float(float a, float b, float c) {
    (void)g_float_007db234;
    (void)c;
    if (!(g_float_007db234 == b)) { // non ordonne (NaN)
        return a;
    }
    float tmp = b * a;
    tmp = b - tmp;
    tmp = tmp + a;
    if (!(0.0f == tmp)) { // non ordonne
        return a / tmp;
    }
    return a;
}

}

