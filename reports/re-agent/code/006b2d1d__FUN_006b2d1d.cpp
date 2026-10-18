#include <cstdint>

struct FUN_006b2d1d_Node {
    std::uint32_t unknown_0[2];
    std::uint32_t link_at_8; // Address of the next node's link field, or zero.
};

struct FUN_006b2d1d_State {
    std::uint32_t first;
    std::uint32_t last;
    std::int32_t count;
};

std::uint32_t __thiscall FUN_006b2d1d(
    FUN_006b2d1d_State* self,
    std::uint32_t param_1,
    std::uint32_t param_2)
{
    std::uint32_t value = self->first;
    std::uint8_t success = 0;

    if (param_1 == value) {
        --self->count;

        if (param_1 == self->last) {
            self->first = 0;
            self->last = 0;
        } else {
            value = reinterpret_cast<FUN_006b2d1d_Node*>(param_1)->link_at_8;
            if (value != 0) {
                value -= 8;
            }
            self->first = value;
        }
    } else {
        if (value == 0) {
            return value;
        }

        if (param_2 == 0) {
            param_2 = value;
        }

        if (reinterpret_cast<FUN_006b2d1d_Node*>(param_2)->link_at_8 != 0) {
            do {
                value = reinterpret_cast<FUN_006b2d1d_Node*>(param_2)->link_at_8;
                const std::uint32_t target_link =
                    (0u - static_cast<std::uint32_t>(param_1 != 0)) & (param_1 + 8u);

                if (value == target_link) {
                    break;
                }

                param_2 = (value == 0) ? 0 : value - 8u;
            } while (reinterpret_cast<FUN_006b2d1d_Node*>(param_2)->link_at_8 != 0);
        }

        value = reinterpret_cast<FUN_006b2d1d_Node*>(param_2)->link_at_8;
        const std::uint32_t target_link =
            (0u - static_cast<std::uint32_t>(param_1 != 0)) & (param_1 + 8u);

        if (value == 0 || value != target_link) {
            return value & 0xffffff00u;
        }

        --self->count;
        value = reinterpret_cast<FUN_006b2d1d_Node*>(param_1)->link_at_8;
        reinterpret_cast<FUN_006b2d1d_Node*>(param_2)->link_at_8 = value;

        if (param_1 == self->last) {
            self->last = param_2;
        }
    }

    success = 1;
    reinterpret_cast<FUN_006b2d1d_Node*>(param_1)->link_at_8 = 0;
    return (value & 0xffffff00u) | success;
}