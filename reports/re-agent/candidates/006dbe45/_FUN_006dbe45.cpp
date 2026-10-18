#include <cstddef>
#include <cstdint>

struct FUN_006dbe45_Result {
    std::uint8_t unknown_00[0x58];
    std::uint32_t field_58;
};

static_assert(offsetof(FUN_006dbe45_Result, field_58) == 0x58);

std::uint32_t __fastcall FUN_006db3f9(
    std::uint32_t* param_1,
    std::uint32_t* param_2,
    std::uint32_t* param_3);

std::uint32_t __cdecl FUN_006dbe45(std::uint32_t* param_1)
{
    std::uint32_t local_c;
    std::uint32_t local_8;

    if (FUN_006db3f9(param_1, &local_c, &local_8) != 0) {
        return 0;
    }

    const auto* result = reinterpret_cast<const FUN_006dbe45_Result*>(
        static_cast<std::uintptr_t>(local_8));
    return result->field_58;
}
