#include <cstdint>

extern std::uint32_t PTR_FUN_0071b650;

// 32-bit target. The stored value is the address of the global, not its contents.
void __fastcall FUN_006de735(std::uint32_t* param_1)
{
    *param_1 = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(&PTR_FUN_0071b650));
}
