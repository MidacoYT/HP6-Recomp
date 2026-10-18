#include <cstdint>

void __thiscall FUN_006d78a8(void* this_, int param_1)
{
    FUN_006d7745(this_, param_1);

    auto* destination = static_cast<std::uint8_t*>(this_);
    destination[0] = 0x7c;

    // Member names and types at these offsets are unresolved.
    *reinterpret_cast<std::uint32_t*>(destination + 0x10) =
        *reinterpret_cast<std::uint32_t*>(param_1 + 0x74);
    *reinterpret_cast<std::uint32_t*>(destination + 0x14) =
        *reinterpret_cast<std::uint32_t*>(param_1 + 0x78);
    *reinterpret_cast<std::uint32_t*>(destination + 0x18) =
        *reinterpret_cast<std::uint32_t*>(param_1 + 0x7c);
    *reinterpret_cast<std::uint32_t*>(destination + 0x1c) =
        *reinterpret_cast<std::uint32_t*>(param_1 + 0x80);

    int size = FUN_006d768d(param_1);
    FUN_00688550(this_, reinterpret_cast<int>(destination + 0x20), size - 0x20);
}
