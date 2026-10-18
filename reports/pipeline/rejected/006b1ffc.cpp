#include <cstdint>

void __thiscall FUN_006b2d1d(void* self,
                             std::uint32_t node,
                             std::uint32_t previous);

std::uint32_t __thiscall FUN_006b1ffc(void* self,
                                      std::int32_t param_1,
                                      char param_2)
{
    std::uint32_t previous = 0;
    std::uint32_t node = *static_cast<std::uint32_t*>(self);

    while (node != 0) {
        if (*reinterpret_cast<std::int32_t*>(node + 0x0c) == param_1) {
            if (param_2 != 0) {
                FUN_006b2d1d(self, node, previous);
            }
            return node;
        }

        previous = node;
        const std::uint32_t link =
            *reinterpret_cast<std::uint32_t*>(node + 0x08);
        node = link == 0 ? 0 : link - 8;
    }

    return 0;
}
