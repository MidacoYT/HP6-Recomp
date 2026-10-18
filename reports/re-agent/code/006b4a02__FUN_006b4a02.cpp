#include <cstdint>

extern void* PTR_FUN_00715d0c;
extern void __cdecl FUN_005dc710(int);

std::uint32_t* __thiscall FUN_006b4a02(void* this_, std::uint8_t param_1)
{
    *reinterpret_cast<void**>(this_) = &PTR_FUN_00715d0c;

    if ((param_1 & 1u) != 0) {
        FUN_005dc710(reinterpret_cast<int>(this_));
    }

    return reinterpret_cast<std::uint32_t*>(this_);
}