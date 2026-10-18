#include <cstdint>

extern std::uint32_t __fastcall FUN_006db3f9(
    std::uint32_t* param_1,
    std::uint32_t* param_2,
    std::uint32_t* param_3);
extern void __cdecl FUN_006b2408(std::uint32_t param_1, char param_2);

void __cdecl FUN_006dbb17(std::uint32_t* param_1, std::int32_t param_2)
{
    std::uint32_t local_c;
    std::uint32_t local_8;

    if (FUN_006db3f9(param_1, &local_c, &local_8) == 0) {
        *reinterpret_cast<std::int32_t*>(local_8 + 0x54) = param_2;

        if (param_2 != 0 &&
            *reinterpret_cast<std::int32_t*>(local_8 + 0x44) == 1) {
            FUN_006b2408(
                *reinterpret_cast<std::uint32_t*>(local_8 + 0x184),
                static_cast<char>(
                    *reinterpret_cast<std::uint32_t*>(local_8 + 0x4c)));
        }
    }
}