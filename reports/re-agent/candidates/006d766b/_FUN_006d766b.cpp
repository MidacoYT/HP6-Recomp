#include <cstdint>

// The member at offset 0x90 has no evidence-backed name or signedness.
int __cdecl FUN_006d766b(int param_1)
{
    auto* member = reinterpret_cast<const std::uint32_t*>(
        static_cast<std::uint32_t>(param_1) + 0x90u);
    return static_cast<std::int32_t>(*member + 0x11u);
}
