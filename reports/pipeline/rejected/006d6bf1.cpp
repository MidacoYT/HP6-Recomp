#include <cstddef>
#include <cstdint>

struct Unresolved_006d6bf1 {
    std::byte unknown_00[4];
    std::uint32_t field_04; // Member name and enclosing type are unresolved.

    void __thiscall FUN_006d6bf1(std::int32_t param_1);
};

void __thiscall Unresolved_006d6bf1::FUN_006d6bf1(std::int32_t param_1)
{
    if (param_1 == 0) {
        field_04 = 0;
    } else {
        // The 32-bit subtraction wraps as it does in the target binary.
        field_04 = static_cast<std::uint32_t>(param_1)
                 - static_cast<std::uint32_t>(
                       reinterpret_cast<std::uintptr_t>(this));
    }
}
