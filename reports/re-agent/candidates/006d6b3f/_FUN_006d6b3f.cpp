#include <cstdint>

extern std::uint32_t* DAT_00895c84;

void __cdecl FUN_006d6b3f(std::uint32_t param_1)
{
    using IndirectFn = void (__cdecl*)(std::uint32_t, std::uint32_t);

    auto* slot = reinterpret_cast<IndirectFn*>(
        static_cast<std::uintptr_t>(*DAT_00895c84) + 4);
    (*slot)(param_1, 0);
}
