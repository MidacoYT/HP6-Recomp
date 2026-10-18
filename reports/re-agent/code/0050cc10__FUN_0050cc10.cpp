#include <cfloat>

static_assert(LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384,
              "This function requires an 80-bit x87 long double.");

long double __fastcall FUN_0050cc10(float* param_1)
{
    return static_cast<long double>(*param_1);
}