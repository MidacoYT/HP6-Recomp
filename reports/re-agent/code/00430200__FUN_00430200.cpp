#include <cfloat>

static_assert(LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384,
              "This function requires an 80-bit x87 long double.");

long double __fastcall FUN_00430200(float** param_1)
{
    // No evidence-backed member name is available for the float at offset 8.
    return static_cast<long double>((*param_1)[2]);
}