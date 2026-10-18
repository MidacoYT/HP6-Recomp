#include <cstddef>
#include <cstdint>

void __thiscall FUN_00688550(
    void* this_,
    std::int32_t param_1,
    std::uint32_t param_2)
{
    auto* base = static_cast<std::byte*>(this_);
    auto& field_8 = *reinterpret_cast<std::uint32_t*>(base + 0x8);
    auto& field_c = *reinterpret_cast<std::uint32_t*>(base + 0xC);

    if (param_1 == 0) {
        field_8 = 0;
        field_c = 0;
        return;
    }

    field_8 = static_cast<std::uint32_t>(param_1)
            - static_cast<std::uint32_t>(
                  reinterpret_cast<std::uintptr_t>(this_));
    field_c = param_2;
}
