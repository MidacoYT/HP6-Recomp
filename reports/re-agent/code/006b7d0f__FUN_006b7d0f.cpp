#include <cstdint>

extern std::uint32_t PTR_LAB_00715e18;
extern std::uint32_t PTR_LAB_00715da8;

void __fastcall FUN_006b7d0f(std::uint32_t* param_1)
{
    using VirtualCall = void (__thiscall*)(void*);

    param_1[0] = reinterpret_cast<std::uint32_t>(&PTR_LAB_00715e18);

    auto call_slot = [](std::uint32_t slot, std::uint32_t offset) {
        auto* object = reinterpret_cast<void*>(slot);
        auto* vtable = *reinterpret_cast<std::uint8_t**>(object);
        auto function = *reinterpret_cast<VirtualCall*>(vtable + offset);
        function(object);
    };

    call_slot(param_1[5], 0x08);
    call_slot(param_1[2], 0x08);
    call_slot(param_1[3], 0x08);
    call_slot(param_1[1], 0x0c);

    param_1[0] = reinterpret_cast<std::uint32_t>(&PTR_LAB_00715da8);
}