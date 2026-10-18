#include <cstdint>

extern "C" std::uint8_t __cdecl FUN_006d7170(std::int32_t param_1);
extern std::uint8_t DAT_0071b150[];

std::uint32_t __cdecl FUN_006d81a4(std::uint32_t param_1)
{
    const std::uint8_t result =
        FUN_006d7170(static_cast<std::int32_t>(param_1));

    if (result == 0 && param_1 != 0x68u) {
        const std::uint32_t value = DAT_0071b150[param_1 & 0xffu];
        if (value == 8u || (value > 0xeu && (value < 0x11u || value == 0x20u))) {
            return 1u;
        }
    }

    // The decompile leaves the upper 24 bits of the callee's return register
    // unresolved when result is nonzero; it explicitly clears only the low byte.
    return 0u;
}
