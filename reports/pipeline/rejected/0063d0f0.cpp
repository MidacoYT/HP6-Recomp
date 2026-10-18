#include <cstdint>

extern std::uint32_t DAT_00c70a78;
extern char DAT_0089cbfe;
extern int DAT_00998580;
extern float _DAT_00b10890;
extern float _DAT_00cef858;
extern std::uint8_t DAT_00ea23e0;
extern std::uint8_t _Dst_00b170d0;
extern std::uint8_t* DAT_00967218;

void __cdecl FUN_0063ccf0(char* format, ...);

void __fastcall FUN_0063d0f0(int param_1)
{
    const std::uint32_t mask =
        1u << (static_cast<std::uint8_t>(param_1) & 0x1f);

    if ((DAT_00c70a78 & mask) != 0) {
        DAT_0089cbfe = static_cast<char>(mask);
        DAT_00998580 = param_1;

        const std::uint32_t index = static_cast<std::uint32_t>(param_1);
        const auto float_address =
            reinterpret_cast<std::uintptr_t>(&DAT_00ea23e0) + index * 0xa0u;
        _DAT_00b10890 =
            *reinterpret_cast<const float*>(float_address) + _DAT_00cef858;

        const auto region_address =
            reinterpret_cast<std::uintptr_t>(&_Dst_00b170d0) + index * 0x10000u;
        DAT_00967218 = reinterpret_cast<std::uint8_t*>(region_address);
        return;
    }

    FUN_0063ccf0(const_cast<char*>(
        "RenderRegionManager::SetActiveRenderRegion - attempt to activate region that is not in use\n"));
}
