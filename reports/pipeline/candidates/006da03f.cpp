#include <cstdint>

void __cdecl FUN_006da03f(char* param_1, std::uint32_t* param_2,
                            std::uint32_t param_3)
{
    struct LocalFrame {
        char* local_18[2];
        std::uint32_t local_10;
        std::uint32_t local_c;
        std::uint32_t local_8;
    } frame;

    frame.local_8 = param_3;
    frame.local_18[0] = param_1;
    frame.local_c = 1;

    std::uint32_t* puVar1 = FUN_006b25b8(param_1);
    FUN_006b2553(FUN_006d9f47, reinterpret_cast<int>(puVar1),
                 100, frame.local_18);

    if (param_2 != nullptr) {
        *param_2 = frame.local_10;
    }
}
