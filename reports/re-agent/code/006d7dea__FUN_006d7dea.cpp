#include <cstdint>

void __cdecl FUN_006d7dea(void* param_1, std::int32_t param_2)
{
    // Field names for these offsets are unresolved in the supplied evidence.
    const auto* data = reinterpret_cast<const std::uint8_t*>(param_2);
    std::uint32_t cursor =
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(param_1));
    void* previous = nullptr;

    if ((data[4] & 0x02) != 0) {
        const std::uint32_t address =
            (*reinterpret_cast<const std::int32_t*>(data + 0x50) == 0)
                ? ((cursor + 0x9fU) & 0xffffff80U) - 0x20U
                : cursor;
        auto* node = reinterpret_cast<void*>(address);

        FUN_006d7cd2(node, param_2);
        cursor = address + FUN_006d74ce(param_2);
        previous = node;
    }

    if ((data[4] & 0x04) != 0) {
        std::uint32_t address = cursor;
        if (*reinterpret_cast<const std::int32_t*>(data + 0x60) == 0) {
            address = ((cursor + 0x2fU) & 0xfffffff0U) - 0x20U;
            if (previous != nullptr) {
                FUN_006d6bf1(previous, static_cast<std::int32_t>(address));
            }
        }
        auto* node = reinterpret_cast<void*>(address);

        FUN_006d778a(node, param_2);
        cursor = address + FUN_006d760a(param_2);
        previous = node;
    }

    if ((data[5] & 0x01) != 0) {
        cursor = (cursor + 0x2fU) & 0xfffffff0U;
        const std::uint32_t address = cursor - 0x20U;
        auto* node = reinterpret_cast<void*>(address);

        if (previous != nullptr) {
            FUN_006d6bf1(previous, static_cast<std::int32_t>(address));
        }
        FUN_006d7827(node, param_2);
        previous = node;
    }

    if ((data[4] & 0x10) != 0) {
        const std::uint32_t address =
            ((cursor + 0x2fU) & 0xfffffff0U) - 0x20U;
        auto* node = reinterpret_cast<void*>(address);

        if (previous != nullptr) {
            FUN_006d6bf1(previous, static_cast<std::int32_t>(address));
        }
        FUN_006d78a8(node, param_2);
        cursor = address + FUN_006d768d(param_2);
        previous = node;
    }

    if ((data[4] & 0x80) != 0) {
        const std::uint32_t aligned = (cursor + 0x1fU) & 0xfffffff0U;
        const std::uint32_t address = aligned - 0x10U;
        auto* node = reinterpret_cast<void*>(address);

        if (previous != nullptr) {
            FUN_006d6bf1(previous, static_cast<std::int32_t>(address));
        }
        FUN_006d793d(node, param_2);
        cursor = aligned + *reinterpret_cast<const std::int32_t*>(data + 0x84);
        previous = node;
    }

    if ((data[4] & 0x20) != 0) {
        const std::uint32_t address =
            ((cursor + 0x1fU) & 0xfffffff0U) - 0x10U;
        auto* node = reinterpret_cast<void*>(address);

        if (previous != nullptr) {
            FUN_006d6bf1(previous, static_cast<std::int32_t>(address));
        }
        FUN_006d79ce(node, param_2);
        previous = node;
    }

    if (previous != nullptr) {
        *reinterpret_cast<std::uint32_t*>(
            static_cast<std::uint8_t*>(previous) + 4) = 0;
    }
}