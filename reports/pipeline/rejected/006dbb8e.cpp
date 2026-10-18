#include <cstddef>
#include <cstdint>

struct FUN_006dbb8e_Record {
    std::uint32_t field_0;
    std::uint32_t field_4;
};

static_assert(offsetof(FUN_006dbb8e_Record, field_4) == 4);

std::uint32_t __fastcall FUN_006db3f9(
    std::uint32_t* param_1,
    std::uint32_t* param_2,
    std::uint32_t* param_3);

std::uint32_t __cdecl FUN_006dbb8e(std::uint32_t* param_1)
{
    std::uint32_t local_c;
    std::uint32_t local_8;

    if (FUN_006db3f9(param_1, &local_8, &local_c) != 0) {
        return 0;
    }

    return reinterpret_cast<FUN_006dbb8e_Record*>(
        static_cast<std::uintptr_t>(local_8))->field_4;
}
