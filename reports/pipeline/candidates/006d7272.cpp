#include <cstdint>

void __thiscall FUN_006d7272(void* this_, std::int32_t param_1)
{
    auto* value = static_cast<std::uint32_t*>(this_);
    *value = (((-static_cast<std::uint32_t>(param_1 != 1) & 0xfffffc00u)
               + 0x400u) * 0x100u)
             | (*value & 0xfffbffffu);
}
