#include "platform/MessageLoop.h"
#include "platform/HandleRegistry.h"
#include "presentation/EngineBootstrap.h"
#include "render/DeviceLostHandler.h"
#include "runtime/FrameClock.h"

#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern std::uint8_t DAT_00cef68d; // WM_QUIT-seen guard
extern std::uint8_t DAT_00866e2a; // "engine is actually running" flag
extern std::uint8_t DAT_00cef68f; // "a fatal error was already shown" flag
extern std::int32_t DAT_00cef55c; // startup-grace-period counter

extern std::uint32_t DAT_00d64570; // real-time accumulator (Q16.16), lo; also in FrameClock.cpp
extern std::int32_t DAT_00d64574;  // real-time accumulator (Q16.16), hi; also in FrameClock.cpp
extern float DAT_0080c680;         // time-scale constant; also in FrameClock.cpp
extern float DAT_0083b230;         // time-scale constant; also in FrameClock.cpp
extern float DAT_008003ac;         // time-scale constant; also in FrameClock.cpp

extern std::uint8_t DAT_00cef69e; // "track performance floor" flag
extern std::int32_t DAT_00866e24; // observed performance-floor minimum

extern std::int32_t DAT_00cef6a0; // a second countdown timer
extern std::uint8_t DAT_00cef69c;
extern std::uint8_t DAT_00cef69d;
extern void* DAT_00fcade0; // also in ChapterLoadStateMachine.cpp

namespace hp6::platform {
namespace {

std::int64_t combine(std::uint32_t lo, std::int32_t hi) {
    return (static_cast<std::int64_t>(hi) << 32) | static_cast<std::uint32_t>(lo);
}

// Same whole/fractional Q16.16-to-float precision idiom as
// hp6::runtime::accumulate_simulation_time_sample, applied here to the
// real-time accumulator (DAT_00d64570/74) instead of the fixed
// simulation step — confirmed identical in the exported x86's own
// FILD/FMUL/FIADD sequence.
float compute_frame_budget_ms() {
    const float fraction = static_cast<float>(DAT_00d64570 & 0xffffu);
    const auto whole = static_cast<float>(static_cast<std::int32_t>(combine(DAT_00d64570, DAT_00d64574) >> 16));
    return (fraction * DAT_0080c680 + whole) * DAT_0083b230 * DAT_008003ac;
}

} // namespace

std::int32_t run_message_loop() {
    if (DAT_00cef68d != 0) {
        DAT_00cef68d = 0;
        return 0;
    }

    MSG message{};
    for (;;) {
        hp6::render::handle_device_lost();

        if (DAT_00866e2a != 0) {
            if (DAT_00cef68f == 0) {
                if (DAT_00cef55c != 0) {
                    --DAT_00cef55c;
                    if (DAT_00cef55c == 0) {
                        hp6::presentation::copy_gsc_message_handles();
                    }
                }
            } else {
                // NOT YET INTEGRATED: a documented deferred no-op —
                // the "a fatal error was already shown" cleanup block
                // (a vtable call on DAT_00fcaf34, FUN_006599b0, a
                // ShowCursor(1) loop, and a one-shot
                // thunk_FUN_0100ea00 hook). Only reached after a fatal
                // error this project's current code never produces.
            }
        }

        message.message = 0;
        const BOOL has_message = PeekMessageA(&message, nullptr, 0, 0, PM_REMOVE | PM_NOYIELD);
        const std::uint32_t accumulator_lo = DAT_00d64570;

        if (has_message == 0) {
            const float budget_ms = compute_frame_budget_ms();
            const DWORD start_tick = GetTickCount();
            (void)accumulator_lo;

            hp6::runtime::run_frame_tick();

            if (DAT_00cef69e != 0) {
                hp6::platform::service_holder();
                // NOT YET INTEGRATED: thunk_FUN_00ff9760 (FUN_005dee50),
                // which tracks a running performance-floor minimum into
                // DAT_00866e24. Deferred as a documented no-op.
            }

            const DWORD end_tick = GetTickCount();
            const std::uint32_t elapsed_ms = end_tick - start_tick;

            if (DAT_00cef6a0 != 0) {
                if (elapsed_ms < static_cast<std::uint32_t>(DAT_00cef6a0)) {
                    DAT_00cef6a0 -= static_cast<std::int32_t>(elapsed_ms);
                } else {
                    DAT_00cef6a0 = 0;
                    if (DAT_00cef69c == 0 && DAT_00fcade0 != nullptr &&
                        *reinterpret_cast<void**>(static_cast<std::uint8_t*>(DAT_00fcade0) + 4) !=
                            nullptr &&
                        **reinterpret_cast<void***>(
                            static_cast<std::uint8_t*>(DAT_00fcade0) + 4) != nullptr) {
                        // NOT YET INTEGRATED: FUN_00689180, a documented
                        // deferred no-op.
                    }
                    if (DAT_00cef69d == 0) {
                        // NOT YET INTEGRATED: FUN_00545f30('\0'), a
                        // documented deferred no-op.
                    }
                }
            }

            if (elapsed_ms != 0 && static_cast<float>(elapsed_ms) < budget_ms) {
                Sleep(0);
            }
        } else {
            if (message.message == WM_QUIT) {
                DAT_00cef68d = 0;
                return static_cast<std::int32_t>(message.wParam);
            }
            if (has_message != -1) {
                TranslateMessage(&message);
                DispatchMessageA(&message);
            }
        }

        if (DAT_00cef68d != 0) {
            break;
        }
    }

    DAT_00cef68d = 0;
    return 0;
}

} // namespace hp6::platform
