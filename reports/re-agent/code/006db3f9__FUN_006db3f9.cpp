#include <cstdint>

std::uint32_t __fastcall FUN_006db3f9(
    std::uint32_t* param_1,
    std::uint32_t* param_2,
    std::uint32_t* param_3)
{
    if (param_1 != nullptr) {
        auto* piVar1 =
            reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(*param_1));
        const std::uint32_t value = *piVar1;

        if (value == 0x4D525453u) {
            *param_2 = static_cast<std::uint32_t>(
                reinterpret_cast<std::uintptr_t>(param_1));
            *param_3 = value;
            return 0;
        }
    }

    return 1;
}