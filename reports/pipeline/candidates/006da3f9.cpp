#include <cstdint>

void __cdecl FUN_006da3f9(char* param_1, std::uint32_t param_2)
{
    struct LocalFrame {
        char* local_18[2];
        std::uint32_t local_10;
        std::uint32_t local_c;
    } frame;

    frame.local_10 = param_2;
    char** ppcVar3 = frame.local_18;
    std::uint32_t uVar2 = 100;
    frame.local_18[0] = param_1;
    frame.local_c = 1;

    std::uint32_t* puVar1 = FUN_006b25b8(param_1);
    FUN_006b2553(FUN_006da296, reinterpret_cast<int>(puVar1),
                 uVar2, ppcVar3);
}
