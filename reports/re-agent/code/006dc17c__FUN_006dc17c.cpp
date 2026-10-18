#include <cstdint>

extern std::int32_t __fastcall FUN_006db3f9(
    std::uint32_t* param_1,
    std::uint32_t* output_1,
    std::uint32_t* output_2);

extern void __cdecl FUN_006dbb17(
    std::uint32_t* param_1,
    std::uint32_t enabled);

void __cdecl FUN_006dc17c(std::uint32_t* param_1, std::int32_t param_2)
{
    std::uint32_t local_c;
    std::uint32_t local_8;

    if (FUN_006db3f9(param_1, &local_c, &local_8) == 0) {
        auto* record = reinterpret_cast<std::uint8_t*>(
            static_cast<std::uintptr_t>(local_8));

        // The record type and member names are unresolved.
        auto& field_50 = *reinterpret_cast<std::int32_t*>(record + 0x50);
        auto& field_58 = *reinterpret_cast<std::int32_t*>(record + 0x58);

        const std::int32_t previous = field_50;
        field_50 = param_2;

        const std::uint32_t enabled =
            static_cast<std::uint32_t>(field_58 < param_2);

        if (static_cast<std::uint32_t>(field_58 < previous) != enabled) {
            FUN_006dbb17(param_1, enabled);
        }
    }
}