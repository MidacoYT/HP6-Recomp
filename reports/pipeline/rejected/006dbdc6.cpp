#include <cstddef>
#include <cstdint>

struct FUN_006dbdc6_Record {
    std::byte unknown_00[0x44];
    std::uint32_t field_44;
};

static_assert(offsetof(FUN_006dbdc6_Record, field_44) == 0x44);

std::uint32_t __fastcall FUN_006db3f9(
    std::uint32_t* param_1,
    std::uint32_t* param_2,
    std::uint32_t* param_3);

std::uint32_t __cdecl FUN_006dbdc6(std::uint32_t* param_1)
{
    std::uint32_t local_c;
    std::uint32_t local_8;

    if (FUN_006db3f9(param_1, &local_c, &local_8) != 0) {
        return 0;
    }

    return reinterpret_cast<FUN_006dbdc6_Record*>(
        static_cast<std::uintptr_t>(local_8))->field_44;
}
