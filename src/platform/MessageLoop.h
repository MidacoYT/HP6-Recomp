#pragma once
#include <cstdint>

namespace hp6::platform {

// 0x005d8fc0. The real Win32 message loop, called once from WinMain
// (0x005d9300) after ShowWindow/UpdateWindow (see
// architecture/startup_path.md). A one-shot guard (DAT_00cef68d, set by
// a prior WM_QUIT) makes a second call return 0 immediately without
// running the loop body at all — matching the exported x86's own early
// return. Each iteration: calls the device-lost handler
// (hp6::render::handle_device_lost, already integrated); when
// DAT_00866e2a is set (the engine is actually running), either counts
// down a startup-grace-period counter (DAT_00cef55c, calling
// hp6::presentation::copy_gsc_message_handles — the real target of
// this call site, confirmed by its address — when it reaches zero) or,
// once a fatal error has already been shown (DAT_00cef68f), runs a
// documented deferred no-op cleanup block instead; then PeekMessageA's.
// With no pending message, computes elapsed real time from the same
// Q16.16 real-time accumulator FrameClock.cpp's
// accumulate_simulation_time_sample reads (DAT_00d64570/74) using the
// identical whole/fractional precision idiom, calls the frame pump
// (hp6::runtime::run_frame_tick, already integrated), lazily registers
// a performance-floor tracker (hp6::platform::service_holder, reused
// rather than duplicated) when DAT_00cef69e is set, and Sleep(0)'s when
// this iteration ran faster than that computed budget. With a pending
// message: returns `wParam` on WM_QUIT (clearing the guard first, same
// as the exported x86), otherwise Translate/DispatchMessage's it
// (skipped entirely when PeekMessageA itself failed, matching the
// BVar3 != -1 check). The loop itself continues until DAT_00cef68d is
// set, at which point it clears the flag and returns 0 — the shared
// exit path every return above also takes implicitly in the exported
// x86 (reproduced here as early returns that still clear the flag
// where the original does).
std::int32_t run_message_loop();

} // namespace hp6::platform
