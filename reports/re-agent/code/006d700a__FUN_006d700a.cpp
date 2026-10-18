#include <cstdint>

// The structure's member names are unresolved. The confirmed fields are
// a type byte at +0, a relative next offset at +4, and a relative value at +8.
std::uint32_t __thiscall FUN_006d700a(void* self, std::int32_t param_1)
{
    auto* current = static_cast<std::uint8_t*>(self);
    std::uint8_t* previous = nullptr;

    while (current != nullptr) {
        if (param_1 == FUN_006d6c09(
                           reinterpret_cast<std::uint32_t*>(current))) {
            break;
        }

        const auto nextOffset =
            *reinterpret_cast<std::uint32_t*>(current + 4);
        previous = current;
        current = nextOffset == 0
            ? nullptr
            : reinterpret_cast<std::uint8_t*>(
                  static_cast<std::uint32_t>(
                      reinterpret_cast<std::uintptr_t>(current)) + nextOffset);
    }

    if (current == nullptr) {
        return 0;
    }

    if (previous == nullptr) {
        current[0] = 0x81;
        if ((current[1] & 0x08) != 0) {
            auto value = *reinterpret_cast<std::uint32_t*>(current + 8);
            if (value != 0) {
                value += static_cast<std::uint32_t>(
                    reinterpret_cast<std::uintptr_t>(current));
            }
            FUN_006d6b3f(value);
        }
    } else {
        const auto nextOffset =
            *reinterpret_cast<std::uint32_t*>(current + 4);
        const auto next = nextOffset == 0
            ? 0u
            : nextOffset + static_cast<std::uint32_t>(
                  reinterpret_cast<std::uintptr_t>(current));

        FUN_006d6bf1(previous, static_cast<std::int32_t>(next));
        FUN_006d81e5(static_cast<std::int32_t>(
            reinterpret_cast<std::uintptr_t>(current)));
    }

    return 1;
}