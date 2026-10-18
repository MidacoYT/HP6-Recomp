#include <cstdint>

void __fastcall FUN_006b4678(std::uint32_t* param_1);

void __fastcall FUN_006b6808(std::uint32_t* param_1)
{
    std::uint32_t* current = param_1;
    for (int count = 0x10; count != 0; --count) {
        *current = 0;
        ++current;
    }

    // Member names for these offsets are unresolved in the supplied evidence.
    param_1[0x10] = 0;
    param_1[0x11] = 0;
    param_1[0x12] = 0;
    param_1[0x13] = 0;

    FUN_006b4678(param_1 + 0x14);
    *reinterpret_cast<std::uint8_t*>(param_1 + 0x18) = 0;
}
