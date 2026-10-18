#include <bit>
#include <cstdint>

std::uint32_t __stdcall FUN_006b1910(std::int32_t param_1)
{
    FUN_006d92a1(DAT_00892b60 + 0x54);

    const std::int32_t base = DAT_00892b60;
    const std::int32_t old_value =
        *reinterpret_cast<std::int32_t*>(DAT_00892b60 + 0x50);
    const std::int32_t new_value = std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(old_value) + 1u);

    *reinterpret_cast<std::int32_t*>(DAT_00892b60 + 0x50) = new_value;
    if (new_value == 0x10000) {
        *reinterpret_cast<std::int32_t*>(base + 0x50) = 1;
    }

    FUN_006d92ac(base + 0x54);

    const std::int32_t offset = std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(param_1) -
        static_cast<std::uint32_t>(
            *reinterpret_cast<std::int32_t*>(DAT_00892b60 + 0x98)));

    // Member names for offsets 0x50, 0x54, and 0x98 are unresolved.
    return static_cast<std::uint32_t>(offset / 0x88) |
           (static_cast<std::uint32_t>(old_value) << 8);
}
