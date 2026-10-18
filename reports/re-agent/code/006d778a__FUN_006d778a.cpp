#include <cstdint>

// Member names and the concrete class type are unresolved in the supplied evidence.
void __thiscall FUN_006d778a(void* self, std::int32_t param_1)
{
    auto* dst = static_cast<std::uint8_t*>(self);
    auto* src = reinterpret_cast<const std::uint8_t*>(param_1);

    FUN_006d7745(self, param_1);
    dst[0] = src[0x54];
    *reinterpret_cast<std::uint32_t*>(dst + 0x18) =
        *reinterpret_cast<const std::uint16_t*>(src + 0x5c);
    *reinterpret_cast<std::uint32_t*>(dst + 0x1c) = 1;

    FUN_006d7368(self, *reinterpret_cast<const std::int32_t*>(src + 0x58));
    *reinterpret_cast<std::uint32_t*>(dst) &= 0x0effffffu;

    const std::int32_t value = FUN_006d760a(param_1);
    FUN_00688550(self, reinterpret_cast<std::int32_t>(dst + 0x20), value - 0x20);
}