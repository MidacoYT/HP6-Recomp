#include <cstddef>
#include <cstdint>

struct FUN_006dbda1_Object {
    std::uint32_t unknown_00[2];
    std::uint32_t field_08;
};

static_assert(offsetof(FUN_006dbda1_Object, field_08) == 8);

std::uint32_t __fastcall FUN_006db3f9(
    std::uint32_t* param_1,
    std::uint32_t* param_2,
    std::uint32_t* param_3);

std::uint32_t __cdecl FUN_006dbda1(std::uint32_t* param_1)
{
    std::uint32_t local_c;
    std::uint32_t local_8;

    if (FUN_006db3f9(param_1, &local_8, &local_c) != 0) {
        return 0;
    }

    return reinterpret_cast<const FUN_006dbda1_Object*>(
        static_cast<std::uintptr_t>(local_8))->field_08;
}
