// Local behavior checks for the frame-pump chain reached from WinMain's
// message loop (0x005e0cc0 and its helpers; see
// architecture/startup_path.md). Game-space symbols and fake vtable-
// shaped objects get test-local definitions, the way the other
// src/render and src/platform smoke tests already do.
#include "runtime/FrameClock.h"

#include <cstdint>
#include <cstdio>

#ifdef _MSC_VER
#define HP6_STDCALL __stdcall
#else
#define HP6_STDCALL
#endif

void* DAT_00cef730 = nullptr;
std::uint8_t DAT_00cef72c = 0;
void* DAT_0089eb0c = nullptr;
void* DAT_00fc7f90 = nullptr;
std::int32_t DAT_0089eb14 = 0;
std::int32_t DAT_00cef790 = 0;
std::uint32_t DAT_00d64570 = 0;
std::int32_t DAT_00d64574 = 0;
std::uint32_t DAT_00d64578 = 0;
std::int32_t DAT_00d6457c = 0;
std::int32_t DAT_00d644f0 = 0;
std::int32_t DAT_00d644f4 = 0;
float DAT_0080c680 = 1.0f;
float DAT_0083b230 = 1.0f;
float DAT_008003ac = 1.0f;
float DAT_00ba9504 = 0.0f;
float DAT_008482ec = 65536.0f;

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

// ---- fake "this"-style vtable objects ---------------------------------
int simulate_calls = 0;
int render_calls = 0;
std::int32_t last_simulate_tick = 0;
std::int32_t last_render_tick = 0;
// Thiscall and stdcall are ABI-compatible here: both are callee-clean for
// explicit stack arguments, and the one explicit argument's position on
// the stack does not depend on whether the (unused) `this` travelled via
// ECX, exactly like tests/pool_list_smoke.cpp's stdcall-declared fake for
// a thiscall-typed vtable slot.
std::int32_t HP6_STDCALL fake_simulate(std::int32_t* tick) {
    ++simulate_calls;
    last_simulate_tick = *tick;
    return 0;
}
std::int32_t HP6_STDCALL fake_render(std::int32_t* tick) {
    ++render_calls;
    last_render_tick = *tick;
    return 0;
}
void* game_object_vtable[2] = {reinterpret_cast<void*>(&fake_simulate),
                               reinterpret_cast<void*>(&fake_render)};
struct FakeGameObject {
    void* vtable = game_object_vtable;
};
FakeGameObject fake_game_object;

int fallback_calls = 0;
std::int32_t* last_fallback_target = nullptr;
std::int32_t HP6_STDCALL fake_fallback(std::int32_t* target) {
    ++fallback_calls;
    last_fallback_target = target;
    return 0;
}
void* fallback_vtable[9] = {};

} // namespace

int main() {
    using namespace hp6::runtime;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // ---- multimedia_timer_elapsed_ms (005e0b90) ---------------------------
    const std::uint32_t t1 = multimedia_timer_elapsed_ms();
    const std::uint32_t t2 = multimedia_timer_elapsed_ms();
    expect("timer_monotonic", t2 >= t1);

    // ---- accumulate_simulation_time_sample + interpolate_render_time ------
    // (005e0a30 / 005e0b10): mode 0 and a unit step/scale make the folded
    // sample exactly 3 per call, so two samples (tick 0 and tick 100,
    // totals 3 and 6) interpolated at tick 50 must land exactly halfway:
    // 3 + (6-3)*0.5 = 4.
    std::uint8_t mode_struct[8] = {};
    *reinterpret_cast<std::int32_t*>(mode_struct + 4) = 0; // mode 0
    DAT_00cef730 = mode_struct;
    DAT_00d64578 = 0x10000; // fixed step = 1.0 (Q16.16)
    DAT_00d6457c = 0;

    std::int32_t tick_a = 0;
    accumulate_simulation_time_sample(&tick_a);
    std::int32_t tick_b = 100;
    accumulate_simulation_time_sample(&tick_b);

    std::int32_t target = 50;
    interpolate_render_time(&target);
    expect("interpolate_channel0", DAT_00d644f0 == 4);
    expect("interpolate_channel1", DAT_00d644f4 == 4);

    // ---- run_frame_tick (005e0cc0) ----------------------------------------
    // Everything else starts at zero, so the simulate- and render-ready
    // checks (accumulator >= threshold, candidate <= threshold) both hold
    // trivially (0 >= 0, 0 <= 0): the very first call must simulate AND
    // render exactly once, never touching the idle fallback.
    DAT_0089eb0c = &fake_game_object;
    fallback_vtable[8] = reinterpret_cast<void*>(&fake_fallback);
    DAT_00fc7f90 = fallback_vtable;
    DAT_00cef790 = 0;
    DAT_00d64570 = 0;
    DAT_00d64574 = 0;

    const std::uint32_t result = run_frame_tick();
    expect("first_tick_simulated", simulate_calls == 1);
    expect("first_tick_rendered", render_calls == 1);
    expect("first_tick_returns_true", (result & 0xffu) == 1);
    expect("fallback_not_called", fallback_calls == 0);

    // ---- initialize_frame_pump_timing (005e0bf0) ---------------------------
    DAT_00ba9504 = 1000.0f;
    DAT_008003ac = 1.0f;
    DAT_0089eb0c = &fake_game_object; // must be cleared by this call
    initialize_frame_pump_timing(60.0f);
    expect("timing_game_object_cleared", DAT_0089eb0c == nullptr);
    expect("timing_step_size_constant", DAT_00d64578 == 0x280000u && DAT_00d6457c == 0);
    // step_value = (1000.0/60.0) * 1.0 = 16.6666...; whole=16, fraction~0.6666...
    // combined Q16.16 = (16<<16) | round(0.6666...*65536) = 0x100000 | 0xAAAB
    const std::uint32_t expected_lo = (16u << 16) | 0xAAABu;
    expect("timing_real_time_accumulator", DAT_00d64570 == expected_lo && DAT_00d64574 == 0);

    if (failures != 0) {
        std::printf("frame_clock_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("frame_clock_smoke: all checks passed\n");
    return 0;
}
