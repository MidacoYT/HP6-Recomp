#include <cstdint>

void __thiscall FUN_006d6bce(void* this_, std::int32_t param_1);
void __thiscall FUN_006d71fd(void* this_, std::int32_t param_1);
void __thiscall FUN_006d71db(void* this_, std::int32_t param_1);

void __thiscall FUN_006d7745(void* this_, std::int32_t param_1)
{
    auto* words = static_cast<std::uint32_t*>(this_);
    const auto* source = reinterpret_cast<const std::int32_t*>(param_1);

    words[0] = (words[0] & 0xFFFFFEFFu) | 0x100u;

    FUN_006d6bce(this_, source[2]); // +0x08
    FUN_006d71fd(this_, source[3]); // +0x0C
    FUN_006d71db(this_, source[4]); // +0x10

    words[2] = 0;
    words[3] = 0;
    words[1] = 0;
}
