#include <cmath>

// Requires an ABI where long double is the x87 80-bit floating-point type.
long double __cdecl FUN_00401020(float param_1)
{
    return std::sqrt(static_cast<long double>(param_1));
}
