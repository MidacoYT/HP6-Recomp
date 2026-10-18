#include <cstdint>

extern void __cdecl FUN_005dc710(std::int32_t);

std::uint32_t* __thiscall FUN_006dcef4(void* self, std::uint8_t param_1)
{
    *reinterpret_cast<void***>(self) =
        reinterpret_cast<void**>(0x0071b520);

    if ((param_1 & 1) != 0) {
        FUN_005dc710(reinterpret_cast<std::int32_t>(self));
    }

    return reinterpret_cast<std::uint32_t*>(self);
}