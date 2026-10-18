#include <cstdint>

extern std::uint8_t __fastcall FUN_006d6c09(std::uint32_t* param_1);

int __fastcall FUN_006d8864(std::uint32_t* param_1)
{
    for (;;) {
        const std::uint32_t value = FUN_006d6c09(param_1);

        if (value == 1) {
            return 0;
        }
        if (value == 2 || value == 4 || value == 0x10) {
            return param_1[3] + 0x20;
        }
        if (value == 0x20 || value == 0x80) {
            return param_1[3] + 0x10;
        }
        if (value != 0x100) {
            return 0;
        }
    }
}
