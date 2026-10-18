#include "runtime/FrameClock.h"

#include <cmath>
#include <cstdlib>
#include <windows.h>
#include <mmsystem.h>
#ifdef _MSC_VER
#include <intrin.h>
#endif

// Addresses read/written by the binary but absent from the exported
// globals; seeded by not-yet-reconstructed code elsewhere (the message
// loop 0x005d8fc0, device/scene setup), so tests provide their own
// definitions the way the other `src/render` and `src/platform` files
// already do.
extern void* DAT_00cef730;   // -> {..., mode:int32 @ +4}; simulation mode (0/1/2)
extern std::uint8_t DAT_00cef72c;  // set to 1 by reset_frame_pump_state; not otherwise read here
extern float DAT_00ba9504;         // interval numerator (see initialize_frame_pump_timing)
extern float DAT_008482ec;         // Q16.16 scale constant (65536.0f in every path observed)
extern void* DAT_0089eb0c;   // opaque "this" for the simulate/render vtable
extern void* DAT_00fc7f90;   // fixed-address object; its own value IS its vtable pointer
extern std::int32_t DAT_0089eb14;  // selects DAT_00d64598 (0) or DAT_00d6459c (1)
extern std::int32_t DAT_00cef790;  // head of the pending visual-effect request queue
extern std::uint32_t DAT_00d64570; // real-time accumulator (Q16.16), lo
extern std::int32_t DAT_00d64574;  // real-time accumulator (Q16.16), hi
extern std::uint32_t DAT_00d64578; // fixed simulation step size (Q16.16), lo
extern std::int32_t DAT_00d6457c;  // fixed simulation step size (Q16.16), hi
extern std::int32_t DAT_00d644f0;  // interpolated render value, channel 0
extern std::int32_t DAT_00d644f4;  // interpolated render value, channel 1
extern float DAT_0080c680;         // time-scale constant
extern float DAT_0083b230;         // time-scale constant
extern float DAT_008003ac;         // time-scale constant

namespace hp6::runtime {
namespace {

#ifdef _MSC_VER
using SimulateOrRenderCall = std::int32_t (__thiscall*)(void*, std::int32_t*);
using FallbackCall = std::int32_t (__thiscall*)(void*, std::int32_t*);
#else
using SimulateOrRenderCall = std::int32_t (*)(void*, std::int32_t*);
using FallbackCall = std::int32_t (*)(void*, std::int32_t*);
#endif

void* vtable_of(void* object) {
    return *reinterpret_cast<void* const*>(object);
}

std::int64_t combine(std::uint32_t lo, std::int32_t hi) {
    return (static_cast<std::int64_t>(hi) << 32) | static_cast<std::uint32_t>(lo);
}

void split(std::int64_t value, std::uint32_t& lo, std::int32_t& hi) {
    lo = static_cast<std::uint32_t>(value);
    hi = static_cast<std::int32_t>(value >> 32);
}

// 0x00fcaf38 / 0x00fcd328: multimedia_timer_elapsed_ms's one-shot baseline.
std::uint8_t DAT_00fcaf38 = 0;
std::uint32_t DAT_00fcd328 = 0;

// 0x00fd16b4 (bit 0) / 0x00fcd320+0x00fcd324: run_frame_tick's one-shot
// baseline for the raw (Q16.16) timer snapshot.
std::uint32_t DAT_00fd16b4 = 0;
std::uint32_t DAT_00fcd320_lo = 0;
std::int32_t DAT_00fcd324_hi = 0;

// 0x00d64580/0x00d64584: the running simulation-time accumulator (Q16.16).
std::uint32_t DAT_00d64580_lo = 0;
std::int32_t DAT_00d64584_hi = 0;

// 0x00d64590/0x00d64594: the next-simulate-time threshold (Q16.16).
std::uint32_t DAT_00d64590_lo = 0;
std::int32_t DAT_00d64594_hi = 0;

// 0x00d64588/0x00d6458c: the render-time baseline (Q16.16).
std::uint32_t DAT_00d64588_lo = 0;
std::int32_t DAT_00d6458c_hi = 0;

// 0x00d64558/0x00d6455c: a 2-slot history of past simulate-time
// thresholds (Q16.16), indexed by the flip flag DAT_0089eb10.
std::int64_t threshold_history[2] = {};

// 0x00d644f8: the current tick value, also read by the (not yet
// reconstructed) renderer.
std::int32_t DAT_00d644f8 = 0;

// 0x00d644fc: written by interpolate_render_time but never read anywhere
// in the exported x86 either; kept only for fidelity.
std::int32_t DAT_00d644fc = 0;

// 0x0089eb10: flip flag selecting which of the two history slots/channel
// buffers below was written last.
std::int32_t DAT_0089eb10 = 0;

// Per-channel (tick, total) sample pairs written by
// accumulate_simulation_time_sample and read by interpolate_render_time.
// Channel 0 tracks DAT_00d64598 (0x00d64500/0x00d64508, flip 0x00d64510);
// channel 1 tracks DAT_00d6459c (0x00d64514/0x00d6451c, flip 0x00d64524).
std::uint32_t total_history[2][2] = {};
std::int32_t tick_history[2][2] = {};
std::int32_t flip_index[2] = {};

// 0x00d64598 / 0x00d6459c: the two running simulation-time totals.
std::int32_t DAT_00d64598 = 0;
std::int32_t DAT_00d6459c = 0;

// 0x0089eb08: the simulation mode last seen by
// accumulate_simulation_time_sample.
std::int32_t DAT_0089eb08 = 0;

// 0x00606b00 — not reconstructed (see FrameClock.h's note on
// drain_pending_visual_effects): a sizeable, independent visual-effect-
// cue subsystem (BLOOM/GLASS/BACKDROP records). Faithful placeholder
// that reports "not completed" so the caller never takes the unlink
// branch; this is dead code while DAT_00cef790 stays null, which is the
// case throughout this project's current coverage.
std::uint32_t process_pending_visual_effect_request(std::int32_t /*node*/) {
    return 0;
}

} // namespace

std::uint32_t multimedia_timer_elapsed_ms() {
    TIMECAPS caps{};
    timeGetDevCaps(&caps, sizeof(caps));
    timeBeginPeriod(caps.wPeriodMin);
    const DWORD now = timeGetTime();
    timeEndPeriod(caps.wPeriodMin);
    if (DAT_00fcaf38 == 0) {
        DAT_00fcaf38 = 1;
        DAT_00fcd328 = now;
    }
    return now - DAT_00fcd328;
}

void drain_pending_visual_effects() {
    if (DAT_00cef790 == 0) {
        return;
    }
    const std::uint32_t start = multimedia_timer_elapsed_ms();
    do {
        if (DAT_00cef790 == 0) {
            return;
        }
        const std::uint32_t result = process_pending_visual_effect_request(DAT_00cef790);
        if ((result & 0xffu) != 0) {
            auto* next_field = reinterpret_cast<std::int32_t*>(
                static_cast<std::uintptr_t>(DAT_00cef790) + 0x7c);
            const std::int32_t next = *next_field;
            *next_field = 0;
            DAT_00cef790 = next;
        }
    } while (multimedia_timer_elapsed_ms() - start < 2);
}

void accumulate_simulation_time_sample(const std::int32_t* tick_value) {
    const std::int32_t mode =
        *reinterpret_cast<const std::int32_t*>(static_cast<const std::uint8_t*>(DAT_00cef730) + 4);

    const float fraction = static_cast<float>(DAT_00d64578 & 0xffffu);
    const std::int32_t whole = static_cast<std::int32_t>(combine(DAT_00d64578, DAT_00d6457c) >> 16);
    const float scaled = (fraction * DAT_0080c680 + static_cast<float>(whole)) * DAT_0083b230 * DAT_008003ac;
    const std::int32_t sample = static_cast<std::int32_t>(scaled) * 3;

    if (mode != 2) {
        DAT_00d6459c += sample;
    }
    if (mode == 0) {
        DAT_00d64598 += sample;
    }

    const int idx0 = (flip_index[0] ^= 1);
    tick_history[0][idx0] = *tick_value;
    total_history[0][idx0] = static_cast<std::uint32_t>(DAT_00d64598);

    const int idx1 = (flip_index[1] ^= 1);
    tick_history[1][idx1] = *tick_value;
    DAT_0089eb08 = mode;
    total_history[1][idx1] = static_cast<std::uint32_t>(DAT_00d6459c);
}

void interpolate_render_time(const std::int32_t* target_time) {
    DAT_00d644fc = *target_time;

    for (int channel = 0; channel < 2; ++channel) {
        const int newest = flip_index[channel];
        const int previous = newest ^ 1;
        const std::int32_t tick_span =
            tick_history[channel][newest] - tick_history[channel][previous];
        float t = 0.0f;
        if (tick_span != 0) {
            const std::int32_t offset = *target_time - tick_history[channel][previous];
            if (offset != 0) {
                t = static_cast<float>(offset) / static_cast<float>(tick_span);
            }
        }
        const auto total_span = static_cast<std::int32_t>(total_history[channel][newest] -
                                                           total_history[channel][previous]);
        const std::int32_t result = static_cast<std::int32_t>(static_cast<float>(total_span) * t) +
                                    static_cast<std::int32_t>(total_history[channel][previous]);
        if (channel == 0) {
            DAT_00d644f0 = result;
        } else {
            DAT_00d644f4 = result;
        }
    }
}

std::uint32_t run_frame_tick() {
    bool simulated = false;
    bool rendered = false;

    drain_pending_visual_effects();

    const std::uint32_t elapsed_ms = multimedia_timer_elapsed_ms();
    const std::int64_t now_fixed = static_cast<std::int64_t>(elapsed_ms) << 16;

    if ((DAT_00fd16b4 & 1u) == 0) {
        DAT_00fd16b4 |= 1u;
        split(now_fixed, DAT_00fcd320_lo, DAT_00fcd324_hi);
    }

    std::int64_t delta = now_fixed - combine(DAT_00fcd320_lo, DAT_00fcd324_hi);
    if (delta > 0x640000) {
        delta = 0x640000;
    }

    std::int64_t accumulator = combine(DAT_00d64580_lo, DAT_00d64584_hi) + delta;
    split(accumulator, DAT_00d64580_lo, DAT_00d64584_hi);
    split(now_fixed, DAT_00fcd320_lo, DAT_00fcd324_hi);

    DAT_00d644f8 = static_cast<std::int32_t>((accumulator * 3) / 0x10000);

    std::int64_t threshold = combine(DAT_00d64590_lo, DAT_00d64594_hi);
    if (accumulator >= threshold) {
        const std::int64_t step = combine(DAT_00d64578, DAT_00d6457c);
        threshold += step;
        const int idx = (DAT_0089eb10 ^= 1);
        threshold_history[idx] = threshold;
        split(threshold, DAT_00d64590_lo, DAT_00d64594_hi);

        std::int32_t sim_tick = static_cast<std::int32_t>((threshold * 3) / 0x10000);
        accumulate_simulation_time_sample(&sim_tick);

        auto* simulate = *reinterpret_cast<SimulateOrRenderCall const*>(vtable_of(DAT_0089eb0c));
        simulate(DAT_0089eb0c, &sim_tick);
        simulated = true;

        accumulator = combine(DAT_00d64580_lo, DAT_00d64584_hi);
    }
    if (accumulator > threshold) {
        threshold = accumulator;
        split(threshold, DAT_00d64590_lo, DAT_00d64594_hi);
    }

    const std::int64_t candidate =
        combine(DAT_00d64570, DAT_00d64574) + combine(DAT_00d64588_lo, DAT_00d6458c_hi);
    if (candidate <= threshold) {
        const int other = DAT_0089eb10 ^ 1;
        std::int64_t render_time = candidate;
        if (render_time < threshold_history[other]) {
            render_time = threshold_history[other];
        }
        split(render_time, DAT_00d64588_lo, DAT_00d6458c_hi);

        std::int32_t render_tick = static_cast<std::int32_t>((render_time * 3) / 0x10000);
        interpolate_render_time(&render_tick);

        auto* render = *reinterpret_cast<SimulateOrRenderCall const*>(
            static_cast<std::uint8_t*>(vtable_of(DAT_0089eb0c)) + 4);
        render(DAT_0089eb0c, &render_tick);
        rendered = true;
    }

    if (simulated && !rendered) {
        // The exported x86 hits an INT3 here: simulating without ever
        // rendering the same step is treated as an invariant violation.
#if defined(_MSC_VER)
        __debugbreak();
#elif defined(__GNUC__) || defined(__clang__)
        __builtin_trap();
#else
        std::abort();
#endif
    }
    if (!simulated && !rendered) {
        auto* fallback = *reinterpret_cast<FallbackCall const*>(
            static_cast<std::uint8_t*>(DAT_00fc7f90) + 0x20);
        std::int32_t* target = (DAT_0089eb14 == 0) ? &DAT_00d64598 : &DAT_00d6459c;
        const std::int32_t result = fallback(&DAT_00fc7f90, target);
        return static_cast<std::uint32_t>(result) & 0xffffff00u;
    }
    return 1u;
}

std::int32_t current_tick_value() {
    return DAT_00d644f8;
}

void reset_frame_pump_state() {
    DAT_00cef72c = 1;
    DAT_0089eb08 = 0;
    DAT_0089eb14 = 0;
    DAT_00d644f8 = 0;
    DAT_00d644fc = 0;
    DAT_00d644f0 = 0;
    DAT_00d644f4 = 0;
    DAT_00d64598 = 0;
    DAT_00d6459c = 0;
    for (int channel = 0; channel < 2; ++channel) {
        tick_history[channel][0] = 0;
        tick_history[channel][1] = 0;
        total_history[channel][0] = 0;
        total_history[channel][1] = 0;
        flip_index[channel] = 0;
    }
}

void initialize_frame_pump_timing(float interval) {
    DAT_00d64580_lo = 0;
    DAT_00d64584_hi = 0;
    DAT_00d64588_lo = 0;
    DAT_00d6458c_hi = 0;
    DAT_00d64590_lo = 0;
    DAT_00d64594_hi = 0;
    threshold_history[0] = 0;
    threshold_history[1] = 0;
    DAT_0089eb10 = 0;
    DAT_0089eb0c = nullptr;

    const float step_value = (DAT_00ba9504 / interval) * DAT_008003ac;
    const auto whole = static_cast<std::int32_t>(step_value); // CVTTSS2SI: truncates toward zero
    const float fraction = step_value - static_cast<float>(whole);
    const auto fraction_fixed = static_cast<std::int64_t>(std::llrint(fraction * DAT_008482ec));

    DAT_00d64578 = 0x280000u;
    DAT_00d6457c = 0;

    const std::int64_t combined = (static_cast<std::int64_t>(whole) << 16) | fraction_fixed;
    split(combined, DAT_00d64570, DAT_00d64574);
}

} // namespace hp6::runtime
