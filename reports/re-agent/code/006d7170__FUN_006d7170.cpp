#include <cstdint>

std::uint8_t __cdecl FUN_006d7170(int param_1)
{
    std::uint8_t result = 0;

    if (param_1 == 0x60 || param_1 == 0x61 || param_1 == 0x62) {
        result = 1;
    }

    return result;
}