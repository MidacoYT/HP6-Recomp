#include <cstdint>

void __thiscall FUN_006d7a48(void* this_, std::int32_t param_1);

void __thiscall FUN_006d7a73(void* this_, std::int32_t param_1)
{
    auto* source = static_cast<std::uint8_t*>(this_);
    auto* destination = reinterpret_cast<std::uint8_t*>(
        static_cast<std::uintptr_t>(static_cast<std::uint32_t>(param_1)));

    // Member names and structure types are unresolved in the supplied evidence.
    auto source32 = [source](std::uintptr_t offset) -> std::uint32_t {
        return *reinterpret_cast<std::uint32_t*>(source + offset);
    };
    auto destination32 = [destination](std::uintptr_t offset) -> std::uint32_t& {
        return *reinterpret_cast<std::uint32_t*>(destination + offset);
    };

    FUN_006d7a48(this_, param_1);

    destination32(0x04) |= 2;
    destination32(0x14) = source32(0x00) & 0xff;
    destination32(0x20) = source32(0x10);
    destination32(0x24) = source32(0x14);
    destination32(0x18) = source32(0x18);
    destination32(0x1c) = source32(0x1c);
    destination32(0x28) = source32(0x00) >> 0x1c;
    destination32(0x2c) = (source32(0x00) >> 0x16) & 1;
    destination32(0x34) = source[2] & 1;
    destination32(0x38) = (source32(0x00) >> 0x11) & 1;
    destination32(0x3c) = (source32(0x00) >> 0x12) & 1;
    destination32(0x40) = (source32(0x00) >> 0x13) & 1;
    destination32(0x44) = (source32(0x00) >> 0x14) & 1;
    destination32(0x48) = (source32(0x00) >> 0x15) & 1;
    destination32(0x4c) = (source32(0x00) >> 0x17) & 1;
    destination32(0x30) = source[3] & 1;
}