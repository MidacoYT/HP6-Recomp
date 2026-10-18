#include <cstdint>

// The class and vtable entry types are unresolved.
extern void* PTR_LAB_00715da8;
extern void __cdecl FUN_005dc710(std::int32_t);

std::uint32_t* __thiscall FUN_006b7cef(void* this_, std::uint8_t param_1)
{
    *reinterpret_cast<void**>(this_) = &PTR_LAB_00715da8;

    if ((param_1 & 1u) != 0) {
        FUN_005dc710(reinterpret_cast<std::int32_t>(this_));
    }

    return reinterpret_cast<std::uint32_t*>(this_);
}
