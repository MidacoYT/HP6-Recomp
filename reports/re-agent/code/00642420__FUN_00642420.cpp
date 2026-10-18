#include <cstdint>

extern std::int32_t DAT_00cf19bc;

extern void __stdcall FUN_006525d0();
extern void __stdcall FUN_006532e0();

void __stdcall FUN_00642420()
{
    FUN_006525d0();

    if (DAT_00cf19bc > 2) {
        FUN_006532e0();
    }
}