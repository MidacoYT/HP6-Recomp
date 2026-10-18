#include <cstdint>

extern std::uint32_t DAT_00892b60;

int __cdecl FUN_006b18df(unsigned int param_1, char param_2)
{
    // The object types and member names at offsets 0x98 and 0x80 are unresolved.
    const auto base = *reinterpret_cast<std::int32_t*>(
        static_cast<std::uintptr_t>(DAT_00892b60 + 0x98u));

    const auto address =
        static_cast<std::uint32_t>((param_1 & 0xffu) * 0x88u) +
        static_cast<std::uint32_t>(base);

    if (*reinterpret_cast<std::int32_t*>(
            static_cast<std::uintptr_t>(address + 0x80u)) != 0 &&
        param_2 != '\0') {
        return *reinterpret_cast<std::int32_t*>(
            static_cast<std::uintptr_t>(DAT_00892b60 + 0x98u));
    }

    return static_cast<std::int32_t>(address);
}
