#include <cstdint>

extern std::uint32_t __fastcall FUN_006db3f9(
    std::uint32_t* param_1,
    std::uint32_t* param_2,
    std::uint32_t* param_3);

void __cdecl FUN_006dbaed(
    std::uint32_t* param_1,
    std::uint32_t param_2,
    std::uint32_t param_3)
{
    std::uint32_t local_c;
    std::uint32_t local_8;

    if (FUN_006db3f9(param_1, &local_c, &local_8) == 0) {
        *reinterpret_cast<std::uint32_t*>(local_8 + 0x48) = param_2;
        *reinterpret_cast<std::uint32_t*>(local_8 + 0x4c) = param_3;
    }
}
