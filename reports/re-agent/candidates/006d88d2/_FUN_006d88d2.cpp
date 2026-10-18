#include <cstdint>

std::uint32_t __fastcall FUN_006d88d2(std::uint32_t* param_1)
{
    std::uint32_t total = 0;
    std::uint32_t* previous = nullptr;

    while (param_1 != nullptr) {
        // The supplied evidence does not identify members at offsets +0x04
        // and +0x0c, so these accesses retain their word offsets.
        const std::uint32_t kind = FUN_006d6c09(param_1);

        if (kind == 2) {
            if (previous != nullptr) {
                total = ((total + 0x7fu) & 0xffffff80u) - 0x20u;
            }
            total = total + 0x20u + param_1[3];
        } else if (kind == 4 || kind == 0x10) {
            if (previous != nullptr) {
                total = (total + 0x0fu) & 0xfffffff0u;
            }
            total = total + 0x20u + param_1[3];
        } else if (kind == 0x20 || kind == 0x80) {
            if (previous != nullptr) {
                total = ((total - 1u) & 0xfffffff0u) + 0x10u;
            }
            total = total + 0x10u + param_1[3];
        } else if (kind == 0x100) {
            if (previous != nullptr) {
                total = (total + 0x0fu) & 0xfffffff0u;
            }
            total = total + FUN_006d8864(param_1);
        } else if (kind != 1) {
            return 0;
        }

        previous = param_1;
        param_1 = param_1[1] == 0
            ? nullptr
            : reinterpret_cast<std::uint32_t*>(
                  reinterpret_cast<std::uintptr_t>(param_1) + param_1[1]);
    }

    return total;
}
