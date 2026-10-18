#pragma once
#include <cstdint>

namespace hp6::runtime {

// 0x005e0b90 (stdcall, no arguments). Returns milliseconds elapsed since
// the first call, using the multimedia timer (timeGetDevCaps/
// timeBeginPeriod/timeGetTime/timeEndPeriod) rather than GetTickCount.
// The first call seeds the baseline (DAT_00fcaf38/_DAT_00fcd328) and
// returns 0.
std::uint32_t multimedia_timer_elapsed_ms();

// 0x005ff790 (stdcall, no arguments). Drains the pending visual-effect
// request queue at DAT_00cef790 (a singly linked list through each
// node's +0x7c field) for up to 2 ms of multimedia-timer time, calling
// 0x00606b00 (process_pending_visual_effect_request, see
// architecture/startup_path.md) on the head node each iteration and
// unlinking it when that call reports completion. A no-op while the
// queue is empty (DAT_00cef790 == 0), which is the case throughout
// every path currently reconstructed in this project — nothing here
// writes DAT_00cef790 non-null yet, so this body is not exercised by
// any test. 0x00606b00 itself is tracked "deferred": it is a sizeable,
// independent visual-effect-cue subsystem (BLOOM/GLASS/BACKDROP
// records) that has not been reverse engineered, so
// process_pending_visual_effect_request() here is a faithful shell
// that preserves 0x005ff790's control flow without performing the
// real per-node work.
void drain_pending_visual_effects();

// 0x005e0a30 (cdecl; `tick_value` is received through EDI in the
// exported x86, not a stack argument — made an explicit parameter
// here). Folds one fixed-point simulation tick's elapsed time
// (computed from DAT_00d64578/_DAT_00d6457c, matching the same
// scaled-fixed-point idiom used by FUN_005d8fc0) into the two running
// totals DAT_00d6459c/DAT_00d64598 (gated by the simulation mode at
// DAT_00cef730+4), then records `*tick_value` and the updated totals
// into the two small double-buffered history arrays used by
// interpolate_render_time().
void accumulate_simulation_time_sample(const std::int32_t* tick_value);

// 0x005e0b10 (cdecl). For each of the 4 tracked channels (stride 0x14
// starting at DAT_00d64500), linearly interpolates between the
// channel's two buffered (tick_value, total) samples to the time
// `*target_time`, writing the result into the DAT_00d644f0[0..3]
// output slots.
void interpolate_render_time(const std::int32_t* target_time);

// 0x005e0cc0 (stdcall, no arguments). The per-iteration "frame pump"
// called from the Win32 message loop (0x005d8fc0) once PeekMessageA
// reports no pending message: drains the visual-effect queue, computes
// elapsed real time since the last call, advances the fixed-point
// simulation accumulator, and — once enough time has accumulated —
// calls the active game/scene object's virtual "simulate" (vtable
// slot 0) and/or "render" (vtable slot 1) method through the opaque
// `this`-style pointer at DAT_0089eb0c (an internal, non-COM vtable:
// confirmed __thiscall from the exported x86, `this` loaded fresh into
// ECX from DAT_0089eb0c immediately before each call — the concrete
// type is not yet identified). When neither fires this call, falls
// back to a third virtual call (vtable slot 8) on the fixed-address
// global object DAT_00fc7f90 (`this` is `&DAT_00fc7f90` itself: its
// own first 4 bytes already hold the vtable pointer). Returns whether
// either the simulate or render call ran this call (matching the
// exported x86's BL-based boolean return).
std::uint32_t run_frame_tick();

// Accessor for DAT_00d644f8 (the current tick value run_frame_tick
// maintains), which other, already-exported game code genuinely reads
// directly (e.g. 0x0059f450/0x00451ec0, part of the
// Task_IdleInputTimer screen task — see
// src/presentation/ScreenTasks.cpp). Kept as a function rather than a
// second extern declaration of the same global, since this file
// already owns that storage.
std::int32_t current_tick_value();

// 0x005e09d0 (stdcall, no arguments). Called once from FUN_005dca10
// (itself called directly from WinMain, right before the real D3D9
// device-creation call — see hp6::render::create_d3d9_device), before
// the message loop or any frame tick has run: sets DAT_00cef72c to 1,
// then zeroes DAT_0089eb08, DAT_0089eb14, DAT_00d644f8, DAT_00d644fc,
// DAT_00d644f0, DAT_00d644f4, DAT_00d64598, DAT_00d6459c, and both
// channels' tick_history/total_history/flip_index entries. A one-time
// reset of every piece of frame-pump state this file owns.
void reset_frame_pump_state();

// 0x005e0bf0 (cdecl, one float argument — called as FUN_005e0bf0(_DAT_008a4474)
// from the engine bootstrap FUN_005dcdf0, itself called once from WinMain
// right after create_and_configure_render_device). Zeroes the running
// simulation-time accumulator (DAT_00d64580/84), the render-time baseline
// (DAT_00d64588/8c), the next-simulate-time threshold (DAT_00d64590/94),
// the 2-slot simulate-time-threshold history (DAT_00d64558/5c/60/64), the
// flip flag (DAT_0089eb10), and DAT_0089eb0c (briefly — FUN_005dcdf0
// reassigns it right after this call returns). Sets the fixed simulation
// step size (DAT_00d64578/7c) to the exact constant 0x280000/0 (40.0 in
// Q16.16, no fractional part — confirmed independent of `interval`) and a
// second Q16.16 value (DAT_00d64570/74) computed from `interval` as
// (DAT_00ba9504 / interval) * DAT_008003ac, split into whole/fractional
// parts and rejoined at 16-bit precision the same way the exported x86
// does (CVTTSS2SI for the whole part, a separate _ftol2-rounded multiply
// by DAT_008482ec for the fractional part, to avoid float32's precision
// loss from multiplying the combined value by 65536 directly).
void initialize_frame_pump_timing(float interval);

} // namespace hp6::runtime
