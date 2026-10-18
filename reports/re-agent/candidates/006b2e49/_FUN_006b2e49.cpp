#include <cstdint>

extern void* PTR_FUN_007159b0;
extern void __cdecl FUN_005dc710(std::int32_t);

std::uint32_t* __thiscall FUN_006b2e49(void* this_, std::uint8_t param_1)
{
    *reinterpret_cast<void**>(this_) = &PTR_FUN_007159b0;

    if ((param_1 & 1u) != 0) {
        FUN_005dc710(static_cast<std::int32_t>(
            reinterpret_cast<std::uintptr_t>(this_)));
    }

    return reinterpret_cast<std::uint32_t*>(this_);
}
