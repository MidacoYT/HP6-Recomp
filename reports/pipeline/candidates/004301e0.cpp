#include <cfloat>

static_assert(LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384,
              "This function requires an 80-bit x87 long double.");

long double __fastcall FUN_004301e0(float** param_1)
{
    // The float at offset 4 has no evidence-backed member name.
    return static_cast<long double>((*param_1)[1]);
}
