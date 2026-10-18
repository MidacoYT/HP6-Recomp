#include <cstdint>

extern std::uint32_t __fastcall FUN_006db3f9(
    std::uint32_t* param_1,
    std::uint32_t* param_2,
    std::uint32_t* param_3);

extern std::uint32_t __fastcall FUN_006db50a(
    std::int32_t param_1,
    std::uint32_t param_2);

std::uint32_t __cdecl FUN_006dbee7(
    std::uint32_t* param_1,
    std::uint32_t param_2)
{
    std::uint32_t local_c;
    std::int32_t local_8;

    if (FUN_006db3f9(param_1, &local_c,
                      reinterpret_cast<std::uint32_t*>(&local_8)) == 0) {
        const std::uint32_t address = FUN_006db50a(local_8, param_2);
        if (address != 0) {
            return *reinterpret_cast<const std::uint32_t*>(address + 4);
        }
    }

    return 0;
}