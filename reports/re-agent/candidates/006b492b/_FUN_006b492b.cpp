#include <cstdint>

using Callback_006b492b = void (__cdecl *)(std::uint32_t, std::uint32_t);
extern Callback_006b492b* DAT_00892df8;

void __cdecl FUN_006b492b(std::uint32_t param_1, std::uint32_t param_2)
{
    DAT_00892df8[1](param_1, param_2);
}
