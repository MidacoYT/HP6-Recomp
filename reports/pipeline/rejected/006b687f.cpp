#include <cstdint>

void __fastcall FUN_006b4678(std::uint32_t* param_1);

// No member names are established for the 16 words or the four-word region
// beginning at offset 0x40.
void __fastcall FUN_006b687f(std::uint32_t* param_1)
{
    std::uint32_t* current = param_1;
    for (int count = 0x10; count != 0; --count) {
        *current = 0;
        ++current;
    }

    FUN_006b4678(param_1 + 0x10);
}
