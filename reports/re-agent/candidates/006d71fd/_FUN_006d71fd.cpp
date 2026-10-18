#include <cstdint>

void __thiscall FUN_006d71fd(void* this_, std::int32_t param_1)
{
    auto* word = static_cast<std::uint32_t*>(this_);
    const std::uint32_t original = *word;

    *word = (((-static_cast<std::uint32_t>(param_1 != 1) & 0xFFFFFFF8u) + 8u)
             * 0x100u)
          | (original & 0xFFFFF7FFu);
}
