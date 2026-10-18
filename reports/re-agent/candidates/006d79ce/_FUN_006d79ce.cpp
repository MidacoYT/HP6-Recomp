#include <cstddef>
#include <cstdint>

void __thiscall FUN_006d79ce(void* self, int param_1)
{
    FUN_006d7745(self, param_1);

    *static_cast<std::uint8_t*>(self) = 0x6f;

    // The member at param_1 + 0x90 has no evidence-backed name.
    const auto value = *reinterpret_cast<const std::uint32_t*>(
        static_cast<std::uintptr_t>(param_1) + 0x90);
    FUN_00688550(
        self,
        static_cast<std::byte*>(self) + 0x10,
        value + 1u);
}
