#include <cstdint>

extern std::int32_t DAT_00cf19a0;
void __cdecl FUN_0063ccf0(char* format, ...);

void __stdcall FUN_01003650()
{
    if (DAT_00cf19a0 > 10) {
        return;
    }

    FUN_0063ccf0(const_cast<char*>("ModeOption:%d:%d by %d (%d bit)\n"));
}
