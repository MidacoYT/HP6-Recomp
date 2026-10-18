#include <cstdint>

void __thiscall FUN_006d7249(void* this_, std::int32_t param_1)
{
    auto* value = static_cast<std::uint32_t*>(this_);
    *value = (((-static_cast<std::uint32_t>(param_1 != 1) & 0xfffffe00u)
               + 0x200u) * 0x100u)
             | (*value & 0xfffdffffu);
}
