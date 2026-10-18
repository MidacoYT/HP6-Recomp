#include <cstdint>

extern void* DAT_00cf1944;
extern std::uint32_t DAT_00cf1a38;

void __cdecl FUN_00653430(
    std::uint32_t param_1,
    std::uint32_t param_2,
    std::int32_t param_3,
    std::int32_t param_4)
{
    using Method = void (__thiscall*)(void*, std::uint32_t, void*);

    auto* record = reinterpret_cast<std::uint8_t*>(
        static_cast<std::uint32_t>(param_4));

    if (param_3 == 0) {
        auto* vtable = *reinterpret_cast<std::uint8_t**>(DAT_00cf1944);
        auto method = *reinterpret_cast<Method*>(vtable + 0x16c);
        method(DAT_00cf1944, param_1, record + 0x2c);
    } else if (*reinterpret_cast<std::uint16_t*>(record + 0x22) ==
               DAT_00cf1a38) {
        *reinterpret_cast<std::uint32_t*>(record + 0x2c) = 0;
    } else {
        auto* vtable = *reinterpret_cast<std::uint8_t**>(DAT_00cf1944);
        auto method = *reinterpret_cast<Method*>(vtable + 0x1a8);
        method(DAT_00cf1944, param_1, record + 0x2c);
    }
}
