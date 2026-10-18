#include <bit>
#include <cstdint>

struct HeaderLayout {
    std::uint8_t unknown_00[0x30];
    std::uint32_t field_30; // Member name unresolved.
    std::int32_t field_34;  // Member name unresolved.
};

extern std::uint32_t __fastcall FUN_006db3f9(
    std::uint32_t* input,
    std::uint32_t* validatedInput,
    std::uint32_t* headerAddress);

int __cdecl FUN_006dbb55(std::uint32_t* param_1, std::int32_t param_2)
{
    std::uint32_t validatedInput;
    std::uint32_t headerAddress;

    if (FUN_006db3f9(param_1, &validatedInput, &headerAddress) == 0 &&
        param_2 > 0) {
        const auto* header = reinterpret_cast<const HeaderLayout*>(
            static_cast<std::uintptr_t>(headerAddress));

        if (param_2 <= header->field_34) {
            const std::uint32_t result =
                (header->field_30 - 0x10u) +
                (static_cast<std::uint32_t>(param_2) * 0x10u);
            return std::bit_cast<std::int32_t>(result);
        }
    }

    return 0;
}
