#include <cstdint>

// The callee names and parameter types remain unresolved.
void __fastcall FUN_006de5e1(std::uint32_t* param_1);
void __cdecl FUN_006dcbc7(std::uint32_t param_1, std::uint32_t param_2);

// 32-bit target.
void* __thiscall FUN_006de615(void* self, std::uint8_t param_1)
{
    FUN_006de5e1(static_cast<std::uint32_t*>(self));

    if ((param_1 & 1) != 0) {
        FUN_006dcbc7(
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(self)),
            0x46c);
    }

    return self;
}
