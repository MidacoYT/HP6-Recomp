#include <cstdint>

void __thiscall FUN_006d7cd2(void* this_, std::int32_t param_1)
{
    FUN_006d7745(this_, param_1);

    auto* dst = static_cast<std::uint8_t*>(this_);
    auto* src = reinterpret_cast<const std::uint8_t*>(param_1);

    dst[0] = src[0x14];
    *reinterpret_cast<std::uint32_t*>(dst + 0x18) =
        *reinterpret_cast<const std::uint16_t*>(src + 0x18);
    *reinterpret_cast<std::uint32_t*>(dst + 0x1c) =
        *reinterpret_cast<const std::uint16_t*>(src + 0x1c);
    *reinterpret_cast<std::uint32_t*>(dst + 0x10) =
        *reinterpret_cast<const std::uint32_t*>(src + 0x20);
    *reinterpret_cast<std::uint32_t*>(dst + 0x14) =
        *reinterpret_cast<const std::uint32_t*>(src + 0x24);

    *reinterpret_cast<std::uint32_t*>(dst) =
        (*reinterpret_cast<const std::uint32_t*>(src + 0x28) << 28) |
        (*reinterpret_cast<std::uint32_t*>(dst) & 0x0fffffffU);

    FUN_006d733f(this_, *reinterpret_cast<const std::int32_t*>(src + 0x30));

    const std::uint32_t result =
        FUN_006d81a4(*reinterpret_cast<const std::uint32_t*>(src + 0x14));
    if (static_cast<std::uint8_t>(result) == 0) {
        dst[2] &= 0xbf;
    } else {
        FUN_006d7368(this_, *reinterpret_cast<const std::int32_t*>(src + 0x2c));
    }

    FUN_006d7220(this_, *reinterpret_cast<const std::int32_t*>(src + 0x34));
    FUN_006d7249(this_, *reinterpret_cast<const std::int32_t*>(src + 0x38));
    FUN_006d7272(this_, *reinterpret_cast<const std::int32_t*>(src + 0x3c));
    FUN_006d729b(this_, *reinterpret_cast<const std::int32_t*>(src + 0x40));
    FUN_006d72c4(this_, *reinterpret_cast<const std::int32_t*>(src + 0x44));
    FUN_006d72ed(this_, *reinterpret_cast<const std::int32_t*>(src + 0x48));
    FUN_006d7316(this_, *reinterpret_cast<const std::int32_t*>(src + 0x4c));

    const std::int32_t size = FUN_006d74ce(param_1);
    FUN_00688550(this_, reinterpret_cast<std::int32_t>(dst + 0x20), size - 0x20);
}
