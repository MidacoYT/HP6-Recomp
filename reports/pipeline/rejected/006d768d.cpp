#include <bit>
#include <cstdint>

int __cdecl FUN_006d768d(int param_1)
{
    const auto address =
        static_cast<std::uintptr_t>(static_cast<std::uint32_t>(param_1));

    // Member names are unresolved; only these offsets are supported by the evidence.
    const auto field_80 =
        *reinterpret_cast<const std::int32_t*>(address + 0x80);
    const auto field_7c =
        *reinterpret_cast<const std::int32_t*>(address + 0x7c);

    const auto result =
        static_cast<std::uint32_t>(field_80) *
        static_cast<std::uint32_t>(field_7c) * 4u + 0x20u;
    return std::bit_cast<std::int32_t>(result);
}
