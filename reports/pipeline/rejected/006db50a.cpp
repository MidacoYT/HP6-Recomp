#include <cstdint>

std::uint32_t __fastcall FUN_006db50a(std::int32_t param_1,
                                       std::uint32_t param_2)
{
    const std::uint32_t index = param_2 & 0xffu;

    // Member names at +0x20 and +0x24 are unresolved.
    if (static_cast<std::int32_t>(index) <
        *reinterpret_cast<std::int32_t*>(param_1 + 0x24)) {
        auto* entry = reinterpret_cast<std::uint32_t*>(
            index * 0x128u +
            *reinterpret_cast<std::uint32_t*>(param_1 + 0x20));

        if (param_2 == entry[0]) {
            return -static_cast<std::uint32_t>(entry[1] != 0) &
                   static_cast<std::uint32_t>(
                       reinterpret_cast<std::uintptr_t>(entry));
        }
    }

    return 0;
}
