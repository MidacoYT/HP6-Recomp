#include <cstdint>

std::uint32_t __fastcall FUN_00404670(float* param_1, float* param_2)
{
    if (param_2[0] == param_1[0] &&
        param_2[1] == param_1[1] &&
        param_2[2] == param_1[2]) {
        return 0;
    }

    return 1;
}