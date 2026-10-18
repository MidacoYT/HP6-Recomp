#include <cstdint>

// Field types and names are unresolved; offsets are retained from the binary.
void __thiscall FUN_006d7a48(void* this_, std::uint8_t* param_1);

void __thiscall FUN_006d7b14(void* this_, std::uint8_t* param_1)
{
    auto* source = static_cast<std::uint8_t*>(this_);

    FUN_006d7a48(this_, param_1);

    *reinterpret_cast<std::uint32_t*>(param_1 + 0x04) |= 4u;
    *reinterpret_cast<std::uint32_t*>(param_1 + 0x54) =
        *reinterpret_cast<std::uint32_t*>(source) & 0xffu;
    *reinterpret_cast<std::uint32_t*>(param_1 + 0x5c) =
        *reinterpret_cast<std::uint32_t*>(source + 0x18);
    *reinterpret_cast<std::uint32_t*>(param_1 + 0x58) =
        (*reinterpret_cast<std::uint32_t*>(source) >> 0x16) & 1u;
}