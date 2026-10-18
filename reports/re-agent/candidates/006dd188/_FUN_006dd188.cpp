#include <cstdint>

extern std::uint32_t PTR_FUN_0071b560;

std::uint32_t __fastcall FUN_006de5b0(std::int32_t);
std::uint32_t __thiscall FUN_006dd00c(void*, std::int32_t);
void __fastcall FUN_006de5e1(std::uint32_t*);

void __fastcall FUN_006dd188(std::uint32_t* param_1)
{
    *param_1 = reinterpret_cast<std::uint32_t>(&PTR_FUN_0071b560);
    FUN_006de5b0(reinterpret_cast<std::int32_t>(param_1));

    // The member at offset 0x464 has no evidence-backed name.
    FUN_006dd00c(
        reinterpret_cast<void*>(param_1[0x119]),
        reinterpret_cast<std::int32_t>(param_1));

    FUN_006de5e1(param_1);
}
