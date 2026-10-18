#include "render/RenderRegionManager.h"
#include "platform/Diagnostics.h"

#include <cstddef>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern std::uint32_t DAT_00c70a78;
extern std::uint32_t DAT_00c75a70;
extern char DAT_0089cbfe;
extern std::int32_t DAT_00998580;
extern float DAT_00ea23e0[]; // per-region table, stride 0xa0 bytes
extern float DAT_00cef858;
extern float DAT_00b10890;
extern std::byte _Dst_00b170d0[]; // per-region slot, stride 0x10000 bytes
extern std::byte* DAT_00967218;

namespace hp6::render {

// ReAgent candidate: reports/pipeline/candidates/0063cd60.cpp.
void assert_region_capacity() {
    const std::uint32_t occupied = DAT_00c70a78 | DAT_00c75a70;
    for (int index = 0; index < 8; ++index) {
        if ((occupied & (1u << (static_cast<std::uint8_t>(index) & 0x1fu))) == 0) {
            return;
        }
    }
    hp6::platform::format_diagnostic_message(
        "RenderRegionManager::AllocateRegionHandle - FATAL out of regions\n");
}

// ReAgent candidate: reports/pipeline/rejected/0063d0f0.cpp. Rejected only
// for declaring the (truly returning) diagnostic without [[noreturn]];
// there is nothing after the call in either this function or the
// diagnostic itself, so the two are observably identical here.
void set_active_render_region(std::int32_t index) {
    const auto mask = 1u << (static_cast<std::uint8_t>(index) & 0x1fu);
    if ((DAT_00c70a78 & mask) == 0) {
        hp6::platform::format_diagnostic_message(
            "RenderRegionManager::SetActiveRenderRegion - attempt to activate "
            "region that is not in use\n");
        return;
    }
    DAT_0089cbfe = static_cast<char>(mask);
    DAT_00998580 = index;
    const auto* region_entry = reinterpret_cast<const std::byte*>(DAT_00ea23e0) +
                               static_cast<std::uint32_t>(index) * 0xa0u;
    DAT_00b10890 = *reinterpret_cast<const float*>(region_entry) + DAT_00cef858;
    DAT_00967218 = _Dst_00b170d0 + static_cast<std::uint32_t>(index) * 0x10000u;
}

} // namespace hp6::render
