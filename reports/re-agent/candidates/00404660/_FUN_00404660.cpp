#include <cfloat>
#include <cmath>

static_assert(LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384,
              "This function requires an 80-bit x87 long double.");

long double __cdecl FUN_00404660(float param_1)
{
    return std::fabs(static_cast<long double>(param_1));
}
