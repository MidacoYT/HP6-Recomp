#include <cstdint>

extern void* DAT_00cf1944;
extern std::uint32_t DAT_00cf19d4;
extern std::uint8_t DAT_00865286;
extern std::uint8_t DAT_00c75a7c;

void __stdcall FUN_00648e10()
{
    const std::uint32_t value =
        (-(std::uint32_t)(DAT_00865286 != 0) & 0xfffffffeu) + 2u;
    const std::uint32_t flags = DAT_00cf19d4 | 0x208u;

    // The vtable owner and entry's declared type are unresolved.
    using VtableEntry = void (__thiscall *)(
        void*, std::uint32_t, std::uint32_t, std::uint32_t,
        std::uint32_t, void*, std::uint32_t);

    const auto* vtable = *reinterpret_cast<std::uint8_t* const*>(DAT_00cf1944);
    const auto entry = *reinterpret_cast<VtableEntry const*>(vtable + 0x68);
    entry(DAT_00cf1944, 0xe0u, flags, 0u, value, &DAT_00c75a7c, 0u);
}