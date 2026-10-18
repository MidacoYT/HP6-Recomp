#include <cstdint>

void __thiscall FUN_006d6bce(void* this_, std::int32_t param_1)
{
    auto* word = static_cast<std::uint32_t*>(this_);
    const std::uint32_t original = *word;

    *word = (((-static_cast<std::uint32_t>(param_1 != 1) & 0xFFFFFFFCu) + 4u)
             * 0x100u)
          | (original & 0xFFFFFBFFu);
}
