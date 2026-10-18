#include <cstddef>
#include <cstdint>

struct FUN_006dbe1d_Overlay {
    std::uint8_t unknown_00[0x38];
    std::uint32_t field_38;
    std::uint8_t unknown_3c[4];
    std::uint32_t field_40;
};

static_assert(offsetof(FUN_006dbe1d_Overlay, field_38) == 0x38);
static_assert(offsetof(FUN_006dbe1d_Overlay, field_40) == 0x40);

std::uint32_t __fastcall FUN_006db3f9(
    std::uint32_t* param_1,
    std::uint32_t* param_2,
    std::uint32_t* param_3);

std::int32_t __cdecl FUN_006dbe1d(std::uint32_t* param_1)
{
    std::uint32_t local_c;
    std::uint32_t local_8;

    if (FUN_006db3f9(param_1, &local_c, &local_8) != 0) {
        return 0;
    }

    const auto* value = reinterpret_cast<const FUN_006dbe1d_Overlay*>(
        static_cast<std::uintptr_t>(local_8));
    return static_cast<std::int32_t>(value->field_40 - value->field_38);
}
