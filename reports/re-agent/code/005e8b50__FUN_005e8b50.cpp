#include <cstdint>

std::uint32_t __fastcall FUN_005e8b50(float* param_1)
{
    if (param_1[0] == 0.0f) {
        return 1;
    }
    if (param_1[1] == 0.0f) {
        return 1;
    }
    if (param_1[2] == 0.0f) {
        return 1;
    }
    return 0;
}