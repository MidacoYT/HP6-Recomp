#include <cstdint>

extern std::uint32_t* __cdecl FUN_006b25b8(char* param_1);

void __stdcall FUN_006b2673(char* param_1)
{
    std::uint32_t* puVar1 = FUN_006b25b8(param_1);
    void* object = reinterpret_cast<void*>(puVar1[0x21]); // Offset 0x84; member name unresolved.
    void** vtable = *reinterpret_cast<void***>(object);

    using Method = void (__thiscall*)(void*);
    reinterpret_cast<Method>(vtable[0x2c / sizeof(void*)])(object);
}