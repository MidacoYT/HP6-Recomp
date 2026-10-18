#include <cstddef>
#include <cstdint>
#include <cstring>

// The call site uses EAX as a pointer after this call, although the
// existing reversal of FUN_0063b850 declares a void return.
extern std::uint32_t* __fastcall FUN_0063b850(
    std::uint32_t param_1, std::int32_t param_2);

extern std::uint32_t* DAT_00cf1920;
extern std::uint32_t DAT_00866368;
extern std::uint32_t DAT_00cf1918;
extern std::uint32_t DAT_00cf191c;
extern std::byte _Dst_00bb0a70[];
extern std::byte _Dst_0096b490[];

std::uint32_t __thiscall FUN_01002890(
    void* self, const std::uint32_t* param_1, char param_2, char param_3)
{
    DAT_00cf1920 = FUN_0063b850(
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(self)),
        0x30);

    for (int i = 0; i != 12; ++i) {
        DAT_00cf1920[i] = param_1[i];
    }

    if (param_2 != 0) {
        DAT_00cf1920[4] = DAT_00866368;
        DAT_00cf1918 =
            static_cast<std::uint32_t>(
                *reinterpret_cast<const std::uint8_t*>(param_1 + 9)) *
            static_cast<std::uint32_t>(
                *reinterpret_cast<const std::uint16_t*>(
                    reinterpret_cast<const std::byte*>(param_1) + 0x26));
        std::memcpy(
            _Dst_00bb0a70,
            reinterpret_cast<const void*>(
                static_cast<std::uintptr_t>(param_1[10])),
            DAT_00cf1918);
    }

    void* copy_result = nullptr;
    if (param_3 != 0) {
        DAT_00cf1920[5] = DAT_00866368;
        DAT_00cf191c =
            static_cast<std::uint32_t>(
                *reinterpret_cast<const std::uint16_t*>(
                    reinterpret_cast<const std::byte*>(param_1) + 0x22))
            << 1;
        copy_result = std::memcpy(
            _Dst_0096b490,
            reinterpret_cast<const void*>(
                static_cast<std::uintptr_t>(param_1[11])),
            DAT_00cf191c);
    }

    return (static_cast<std::uint32_t>(
                reinterpret_cast<std::uintptr_t>(copy_result)) &
            0xffffff00U) | 1U;
}
