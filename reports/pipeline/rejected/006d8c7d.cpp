#include <cstdint>

void __cdecl FUN_006d8c7d(std::uint32_t param_1, std::uint32_t* param_2)
{
    param_2[2] = 0;
    param_2[0] = 1;
    param_2[1] = 1;

    switch (param_1) {
    case 8:
        param_2[0] = 1;
        param_2[1] = 2;
        param_2[2] = 1;
        return;
    case 9:
        param_2[0] = 2;
        param_2[2] = 1;
        param_2[1] = 2;
        return;
    case 10:
        param_2[0] = 4;
        param_2[1] = 2;
        param_2[2] = 1;
        return;
    case 11:
        param_2[0] = 8;
        param_2[1] = 2;
        param_2[2] = 1;
        return;
    case 12:
        param_2[0] = 1;
        break;
    case 13:
        param_2[0] = 2;
        break;
    case 14:
        param_2[0] = 4;
        break;
    case 15:
        param_2[0] = 8;
        break;
    default:
        return;
    }

    param_2[2] = 2;
    param_2[1] = 2;
}
