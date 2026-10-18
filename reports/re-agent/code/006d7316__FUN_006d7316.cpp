#include <cstdint>

void __thiscall FUN_006d7316(void* this_, std::int32_t param_1)
{
    auto* value = static_cast<std::uint32_t*>(this_);
    *value = (((-static_cast<std::uint32_t>(param_1 != 1) & 0xffff8000u)
               + 0x8000u) * 0x100u)
             | (*value & 0xff7fffffu);
}