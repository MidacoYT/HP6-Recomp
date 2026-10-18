#include <cstdint>

void __cdecl FUN_006dcbc7(void* param_1, std::uint32_t param_2);

std::uint32_t* __thiscall FUN_006de7a1(void* self, std::uint8_t param_1)
{
    *reinterpret_cast<void***>(self) =
        reinterpret_cast<void**>(0x0071b650);

    if ((param_1 & 1) != 0) {
        FUN_006dcbc7(self, 0x464);
    }

    return static_cast<std::uint32_t*>(self);
}