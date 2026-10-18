#include <cstdint>

extern std::uint32_t DAT_00c70a78;
extern std::uint32_t _DAT_00c75a70;
void __cdecl FUN_0063ccf0(char* format, ...);

void __stdcall FUN_0063cd60()
{
    const std::uint32_t occupied = DAT_00c70a78 | _DAT_00c75a70;

    int index = 0;
    do {
        if ((occupied & (1u << (static_cast<std::uint8_t>(index) & 0x1f))) == 0) {
            return;
        }
        ++index;
    } while (index < 8);

    FUN_0063ccf0(
        const_cast<char*>("RenderRegionManager::AllocateRegionHandle - FATAL out of regions\n"));
}
