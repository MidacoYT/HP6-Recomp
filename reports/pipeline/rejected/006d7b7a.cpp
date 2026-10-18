#include <cstdint>

// The supplied evidence does not identify the owning type or member names.
// The address-based offsets are retained rather than assigned speculative names.
void __thiscall FUN_006d7b7a(void* this_, std::uint8_t* param_1)
{
    FUN_006d7a48(this_, param_1);

    *reinterpret_cast<std::uint32_t*>(param_1 + 0x04) |= 0x10u;
    *reinterpret_cast<std::uint32_t*>(param_1 + 0x74) =
        *reinterpret_cast<std::uint32_t*>(static_cast<std::uint8_t*>(this_) + 0x10);
    *reinterpret_cast<std::uint32_t*>(param_1 + 0x78) =
        *reinterpret_cast<std::uint32_t*>(static_cast<std::uint8_t*>(this_) + 0x14);
    *reinterpret_cast<std::uint32_t*>(param_1 + 0x7c) =
        *reinterpret_cast<std::uint32_t*>(static_cast<std::uint8_t*>(this_) + 0x18);
    *reinterpret_cast<std::uint32_t*>(param_1 + 0x80) =
        *reinterpret_cast<std::uint32_t*>(static_cast<std::uint8_t*>(this_) + 0x1c);
}
