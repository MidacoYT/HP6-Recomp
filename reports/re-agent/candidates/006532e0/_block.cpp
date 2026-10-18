#include <cstdint>

extern std::uint8_t DAT_00f38b10[];

void __stdcall FUN_006534a0(std::int32_t, std::int32_t*);
void __stdcall FUN_00653430(std::uint32_t, std::uint32_t,
                            std::int32_t, std::int32_t);

void __stdcall FUN_006532e0()
{
    for (std::uint32_t i = 0; i < 0x600; ++i) {
        auto* entry = DAT_00f38b10 + i * 0xc4;
        if (*reinterpret_cast<std::int32_t*>(entry + 0x18) != 0) {
            FUN_006534a0(
                *reinterpret_cast<std::int32_t*>(entry + 0x14),
                *reinterpret_cast<std::int32_t**>(entry + 0x2c));
        }
    }

    for (std::uint32_t i = 0; i < 0x600; ++i) {
        auto* entry = DAT_00f38b10 + i * 0xc4;
        if (*reinterpret_cast<std::int32_t*>(entry + 0x18) != 0) {
            FUN_00653430(
                *reinterpret_cast<std::uint32_t*>(entry + 0x18),
                *reinterpret_cast<std::uint32_t*>(entry + 0x1c),
                *reinterpret_cast<std::int32_t*>(entry + 0x14),
                static_cast<std::int32_t>(
                    reinterpret_cast<std::uintptr_t>(entry)));
        }
    }
}
