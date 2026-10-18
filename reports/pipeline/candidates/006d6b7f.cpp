#include <cstdint>

std::uint8_t __cdecl FUN_006d6b7f(std::int32_t param_1)
{
    std::uint8_t result = 0;

    if (param_1 > 0x28 &&
        (param_1 < 0x2b ||
         param_1 == 0x2d ||
         param_1 == 0x2f ||
         param_1 == 0x32)) {
        result = 1;
    }

    return result;
}
