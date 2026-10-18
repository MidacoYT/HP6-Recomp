// Local behavior checks for batch mixed_deps_054: RenderRegionManager,
// ShaderManager and the display-mode enumeration log line. Game-space
// symbols get test-local definitions.
#include "platform/Diagnostics.h"
#include "render/DisplayMode.h"
#include "render/RenderRegionManager.h"
#include "render/ShaderManager.h"

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

std::uint32_t DAT_00c70a78 = 0;
std::uint32_t DAT_00c75a70 = 0;
char DAT_0089cbfe = 0;
std::int32_t DAT_00998580 = -1;
float DAT_00ea23e0[(0xa0 / 4) * 4] = {};
float DAT_00cef858 = 0.0f;
float DAT_00b10890 = 0.0f;
std::byte _Dst_00b170d0[0x10000 * 4] = {};
std::byte* DAT_00967218 = nullptr;
std::uint16_t DAT_009978e8[8] = {};
std::int32_t DAT_00bb0994 = 0;
std::int32_t DAT_00c754cc = 0;
std::int32_t DAT_00cf1a34 = 0;
std::int32_t DAT_00cf19a0 = 0;
std::int32_t DAT_00866368 = 0;
std::int32_t DAT_00cf19a8 = 0;

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}
} // namespace

int main() {
    using namespace hp6::render;

    // ---- hp6::platform::format_diagnostic_message (0063ccf0) ------------
    // The exported x86 formats into a local buffer and discards it; there
    // is nothing externally observable beyond "does not crash".
    hp6::platform::format_diagnostic_message("probe %d %s\n", 7, "x");

    // ---- assert_region_capacity (0063cd60) -------------------------------
    // Likewise has no observable effect of its own in either branch; the
    // call must simply not crash for a free and for a fully-occupied mask.
    DAT_00c70a78 = 0;
    DAT_00c75a70 = 0;
    assert_region_capacity();
    DAT_00c70a78 = 0xff;
    DAT_00c75a70 = 0;
    assert_region_capacity();
    DAT_00c70a78 = 0;
    DAT_00c75a70 = 0;

    // ---- set_active_render_region (0063d0f0) -----------------------------
    DAT_00c70a78 = 1u << 3; // region 3 is in use
    DAT_00ea23e0[3 * (0xa0 / 4)] = 1.5f; // region 3's float field
    DAT_00cef858 = 0.25f;
    DAT_00998580 = -1;
    DAT_0089cbfe = 0;
    DAT_00967218 = nullptr;
    set_active_render_region(3);
    expect("region_index", DAT_00998580 == 3);
    expect("region_mask_byte", DAT_0089cbfe == static_cast<char>(1u << 3));
    expect("region_float", DAT_00b10890 == 1.75f);
    expect("region_pointer", DAT_00967218 == _Dst_00b170d0 + 3u * 0x10000u);

    DAT_00998580 = -1;
    set_active_render_region(5); // region 5 is NOT in use: no-op
    expect("region_rejected_noop", DAT_00998580 == -1);

    // ---- allocate_shader_id / free_shader_id (01003ea0 / 01003f20) -------
    DAT_009978e8[0] = 10;
    DAT_009978e8[1] = 11;
    DAT_00bb0994 = 0;
    DAT_00c754cc = 10;
    DAT_00cf1a34 = 0;

    const auto id1 = allocate_shader_id();
    expect("alloc_id1", id1 == 10u);
    expect("alloc_stack_advanced", DAT_00bb0994 == 1);
    expect("alloc_high_water", DAT_00c754cc == 11);
    expect("alloc_live_count", DAT_00cf1a34 == 1);

    const auto id2 = allocate_shader_id();
    expect("alloc_id2", id2 == 11u);
    expect("alloc_high_water_unchanged", DAT_00c754cc == 12);
    expect("alloc_live_count2", DAT_00cf1a34 == 2);

    free_shader_id(static_cast<std::uint16_t>(id2));
    expect("free_stack_restored", DAT_00bb0994 == 1 && DAT_009978e8[1] == id2);
    expect("free_high_water_lowered", DAT_00c754cc == 11);
    expect("free_live_count", DAT_00cf1a34 == 1);

    free_shader_id(static_cast<std::uint16_t>(id1));
    expect("free_id1_stack", DAT_00bb0994 == 0 && DAT_009978e8[0] == id1);
    expect("free_id1_high_water_lowered", DAT_00c754cc == 10);
    expect("free_id1_live_count", DAT_00cf1a34 == 0);

    // Freeing above the high-water mark must leave every global untouched.
    DAT_00bb0994 = 0;
    DAT_00c754cc = 5;
    DAT_00cf1a34 = 0;
    free_shader_id(99);
    expect("free_above_mark_noop", DAT_00bb0994 == 0 && DAT_00c754cc == 5 &&
                                   DAT_00cf1a34 == 0);

    // ---- report_mode_option (01003650) -------------------------------------
    // No observable effect regardless of branch (format_diagnostic_message
    // discards its buffer); only a crash-free call can be checked here.
    DAT_00cf19a0 = 0;
    DAT_00866368 = 1;
    DAT_00cf19a8 = 32;
    report_mode_option(1024, 768);
    DAT_00cf19a0 = 11;
    report_mode_option(1024, 768); // capped: no-op

    if (failures != 0) {
        std::printf("render_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("render_smoke: all checks passed\n");
    return 0;
}
