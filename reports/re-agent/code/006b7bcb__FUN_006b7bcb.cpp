#include <cstdint>

extern std::uint32_t PTR_LAB_00715d30;
extern std::uint32_t PTR_FUN_00715d0c;
extern void __fastcall FUN_006b7d0f(std::uint32_t* param_1);

void __fastcall FUN_006b7bcb(std::uint32_t* param_1)
{
    param_1[0] = reinterpret_cast<std::uint32_t>(&PTR_LAB_00715d30);
    param_1[7] = reinterpret_cast<std::uint32_t>(&PTR_FUN_00715d0c);
    FUN_006b7d0f(param_1);
}