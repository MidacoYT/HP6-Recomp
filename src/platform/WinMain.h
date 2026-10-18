#pragma once
#include <windows.h>
#include <cstdint>

namespace hp6::platform {

// Injectable in place of the real TerminateProcess, so a test can
// observe that WinMain reached its success/failure exit points without
// the test process actually dying. The exported x86 never returns past
// a successful call to this (TerminateProcess does not return); when
// the injected callback chooses not to end the process either, this
// reconstruction returns instead of hanging, so it stays testable.
using ProcessTerminationCall = void (*)(void* process, std::uint32_t exit_code);

void real_terminate_process(void* process, std::uint32_t exit_code); // kernel32 TerminateProcess

// 0x005d9300. The game's real WinMain, assembled from every piece this
// project has reconstructed and tested in isolation (see
// architecture/startup_path.md for the full narrative). Control flow,
// in order:
//   1. FPU control word, exe-relative current directory, three
//      SystemParametersInfoA saves (hp6::platform::
//      apply_system_parameters(false, ...) is the real FUN_005d9210
//      call at this point), copies `command_line` into two fixed
//      buffers, and a documented deferred "-key=value" command-line
//      parser (parse_command_line_overrides).
//   2. A single-instance check (FindWindowA
//      "HalfBloodPrinceMainWndClass"); if another instance is already
//      running, terminates immediately with exit code 0.
//   3. Otherwise: registers the window class
//      (hp6::platform::register_game_window_class), reads ~27
//      GameSettings registry values (resolution/quality settings, six
//      Mode%dWidth/Height pairs, window position/size/Maximized/
//      Minimized), creates the real game window
//      (hp6::platform::create_game_window), and — on success — creates
//      the D3D9 device (hp6::render::create_and_configure_render_device),
//      brings up file-system storage, particle tables, the (deferred)
//      debug-variable and runtime-services subsystems, the engine
//      bootstrap (hp6::presentation::initialize_engine_core_systems),
//      and checks the hardware-compatibility gate
//      (hp6::platform::is_platform_compatible).
//   4. On a passing gate: dispatches TriggerBootupFlow
//      (hp6::presentation::trigger_bootup_flow), shows/updates the
//      window, runs the real message loop
//      (hp6::platform::run_message_loop) to completion, tears down the
//      render device (hp6::render::shutdown_render_device) and the
//      remaining engine-service hooks, restores the saved
//      SystemParametersInfoA values, and terminates the process with
//      exit code 1 — the success path, reached once ScreenLanguageSelect
//      (or whatever chapter TriggerBootupFlow resolves) has already run
//      to the user closing the window.
//   5. On any earlier failure (RegisterClass, window creation, or the
//      compatibility gate), a single shared fallback destroys the
//      window if one exists, restores the saved SystemParametersInfoA
//      values directly, and returns the configured window width — the
//      exported x86's own fall-through-past-nested-ifs shape,
//      reproduced here with the identical nested-if structure rather
//      than early returns, since the real binary reaches this same
//      code from three different failure points by simple fall-through.
// `terminate` defaults to the real TerminateProcess; tests substitute a
// recording stub so the message loop's own result can be observed
// afterward instead of the process actually dying.
std::int32_t run_win_main(HINSTANCE instance, char* command_line,
                          ProcessTerminationCall terminate = &real_terminate_process);

} // namespace hp6::platform
