#include <cstdint>

std::uint32_t __fastcall FUN_006db3f9(
    std::uint32_t* param_1,
    std::uint32_t* param_2,
    std::uint32_t* param_3);

std::uint32_t __cdecl FUN_006dbdeb(std::uint32_t* param_1)
{
    std::uint32_t local_c;
    std::uint32_t local_8;

    const std::uint32_t result = FUN_006db3f9(param_1, &local_c, &local_8);
    if (result != 0) {
        return result & 0xffffff00u;
    }

    // The supplied evidence does not identify members at these offsets.
    if (*reinterpret_cast<const std::uint32_t*>(local_8 + 0x44u) == 0 &&
        *reinterpret_cast<const std::uint32_t*>(local_c + 0x08u) == 0) {
        return 1;
    }
    return 0;
}
