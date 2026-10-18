#include <cstdint>

void __fastcall FUN_006b7d0f(std::uint32_t* param_1);
void __cdecl FUN_006b492b(std::uint32_t param_1, std::uint32_t param_2);

std::uint32_t* __thiscall FUN_006b7dcb(void* self, std::uint8_t param_1)
{
    FUN_006b7d0f(static_cast<std::uint32_t*>(self));

    if ((param_1 & 1) != 0) {
        FUN_006b492b(reinterpret_cast<std::uint32_t>(self), 0x1c);
    }

    return static_cast<std::uint32_t*>(self);
}
