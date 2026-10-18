#include <cstdint>

extern char DAT_0071b150[];
extern std::uint8_t __cdecl FUN_006d6b7f(std::int32_t);

std::uint32_t __cdecl FUN_006d6ba4(std::uint32_t param_1)
{
    const std::uint32_t index = param_1 & 0xffu;
    std::uint8_t result = 0;

    if (DAT_0071b150[index] != 0) {
        if (FUN_006d6b7f(static_cast<std::int32_t>(param_1)) == 0) {
            result = 1;
        }
    }

    // The decompile does not establish the upper 24 return bits after the call.
    return result;
}