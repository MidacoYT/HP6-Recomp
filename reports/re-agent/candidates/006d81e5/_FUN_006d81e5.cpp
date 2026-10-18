#include <cstdint>

// FUN_006d6b3f (0x006d6b3f); its parameter type is unresolved.
void __cdecl FUN_006d6b3f(std::uint32_t param_1);

void __cdecl FUN_006d81e5(std::int32_t param_1)
{
    const auto base = static_cast<std::uint32_t>(param_1);
    const auto* bytes = reinterpret_cast<const std::uint8_t*>(base);

    // The member names at offsets 1, 4, and 8 are unresolved.
    const auto firstOffset =
        *reinterpret_cast<const std::uint32_t*>(bytes + 4);
    const std::uint32_t firstAddress =
        firstOffset == 0 ? 0 : firstOffset + base;

    if (firstAddress != 0) {
        FUN_006d81e5(static_cast<std::int32_t>(firstAddress));
    }

    if ((bytes[1] & 8) != 0) {
        std::uint32_t secondAddress =
            *reinterpret_cast<const std::uint32_t*>(bytes + 8);
        if (secondAddress != 0) {
            secondAddress += base;
        }
        FUN_006d6b3f(secondAddress);
    }

    if ((bytes[1] & 4) != 0) {
        FUN_006d6b3f(base);
    }
}
