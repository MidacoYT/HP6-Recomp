#include <cstdint>

extern std::uint32_t PTR_FUN_0071b628;
void __cdecl FUN_006dcbc7(void* ptr, std::uint32_t size);

std::uint32_t* __thiscall FUN_006de65f(void* this_, std::uint8_t param_1)
{
    *reinterpret_cast<std::uint32_t**>(this_) = &PTR_FUN_0071b628;

    if ((param_1 & 1) != 0) {
        FUN_006dcbc7(this_, 0x24);
    }

    return static_cast<std::uint32_t*>(this_);
}
