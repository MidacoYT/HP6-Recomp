#include <bit>
#include <cstdint>

int __cdecl FUN_006d767c(int param_1)
{
    // The member at offset 0x84 has no evidence-backed name.
    const auto address = static_cast<std::uint32_t>(param_1) + 0x84u;
    const auto value = *reinterpret_cast<const std::uint32_t*>(address);
    return std::bit_cast<std::int32_t>(value + 0x10u);
}
