#pragma once
#include <cstdint>

namespace hp6::render {

// 0x0063cd60 (cdecl). Scans bits 0-7 of (DAT_00c70a78 | DAT_00c75a70) for
// a clear bit (a free region slot) and returns as soon as one is found.
// This function does not mark anything in use itself — it is a
// precondition guard only. When all 8 bits are set, it calls the fatal
// diagnostic "RenderRegionManager::AllocateRegionHandle - FATAL out of
// regions" and returns (see hp6::platform::format_diagnostic_message for
// why the exported noreturn tag does not matter here).
void assert_region_capacity();

// 0x0063d0f0 (fastcall). When bit `index` of DAT_00c70a78 is set (the
// region is in use), records the index at DAT_00998580, the mask's low
// byte at DAT_0089cbfe, sums a per-region float from the 0xa0-byte-stride
// table at DAT_00ea23e0 with DAT_00cef858 into DAT_00b10890, and points
// DAT_00967218 at the 0x10000-byte-stride slot of _Dst_00b170d0 for this
// region. Otherwise calls the fatal diagnostic
// "RenderRegionManager::SetActiveRenderRegion - attempt to activate
// region that is not in use" and returns without touching any of those
// globals.
void set_active_render_region(std::int32_t index);

} // namespace hp6::render
