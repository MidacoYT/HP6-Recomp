#include <cstdint>

// The member at 0x468 and the two virtual methods have no resolved names.
std::uint32_t __fastcall FUN_006de5b0(int param_1)
{
    auto* field_468 = reinterpret_cast<void**>(param_1 + 0x468);
    void* object = *field_468;

    if (object != nullptr) {
        auto* vtable = *reinterpret_cast<void***>(object);
        reinterpret_cast<void(__thiscall*)(void*)>(vtable[0x20 / 4])(object);

        object = *field_468;
        if (object != nullptr) {
            vtable = *reinterpret_cast<void***>(object);
            reinterpret_cast<void(__thiscall*)(void*)>(vtable[0x08 / 4])(object);
            *field_468 = nullptr;
        }
    }

    return 0;
}