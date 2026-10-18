#include <cstdint>

void __cdecl FUN_006d9f1a(char* param_1)
{
    char* local_18[3];
    std::uint32_t local_c = 0;

    char** ppcVar3 = local_18;
    std::uint32_t uVar2 = 100;
    local_18[0] = param_1;

    std::uint32_t* puVar1 = FUN_006b25b8(param_1);
    FUN_006b2553(FUN_006d9eac, reinterpret_cast<int>(puVar1), uVar2, ppcVar3);
}
