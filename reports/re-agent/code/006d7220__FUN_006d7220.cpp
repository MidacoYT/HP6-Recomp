#include <cstdint>

// The first 32-bit word of this_ptr has no evidence-backed member name.
void __thiscall FUN_006d7220(void* this_ptr, std::int32_t param_1)
{
    auto* word = static_cast<std::uint32_t*>(this_ptr);
    const std::uint32_t preserved = *word & 0xfffeffffu;
    const std::uint32_t value =
        ((-static_cast<std::uint32_t>(param_1 != 1) & 0xffffff00u) + 0x100u) * 0x100u;
    *word = value | preserved;
}