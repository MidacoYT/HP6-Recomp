#include <cstdint>

// The supplied evidence does not identify the members at these offsets.
void __fastcall FUN_006d73b8(std::uint32_t* param_1)
{
    for (std::int32_t i = 0; i < 0x21; ++i) {
        param_1[i] = 0;
    }

    param_1[0x21] = 0x40;
    param_1[0x23] = 0;
    param_1[0x24] = 0;
    param_1[0x22] = 0;

    std::uint32_t* p = param_1 + 0x25;
    for (std::int32_t i = 0xb; i != 0; --i) {
        *p = 0;
        ++p;
    }
}
