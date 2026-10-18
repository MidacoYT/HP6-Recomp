#pragma once
#include <cstdint>

namespace hp6::platform {

// 0x006b3a70. Called once from WinMain, right after
// initialize_file_system_storage. One-shot guarded (DAT_00892df0) table
// reset for a particle/effects subsystem: zeroes four partially-
// overlapping byte/dword fields across the fixed static range
// [0x00851450, 0x008516ad) (confirmed by reading the raw byte-stride
// loops directly), then calls two leaf functions (FUN_006b39e2(2,1),
// FUN_006b3950(0,1)). Nothing this project has traced reads that range
// back, so the zero-fills and the two leaf calls are a documented
// deferred no-op; only the one-shot guard itself is reproduced, to keep
// a second call's early-return behaviour faithful.
void reset_particle_system_tables();

// 0x00625ce0. Called once from WinMain, right after
// reset_particle_system_tables. Initializes a large debug/developer
// console-variable system (writes ~30 fields at DAT_00d6b8xx, registers
// a "DEBUG" category through FUN_005ef120/FUN_005ef020, and allocates a
// variable slot via FUN_0063c310). Purely a developer-tooling subsystem:
// nothing this project's render, chapter-flow or message-loop code
// reads any DAT_00d6b8xx field. Documented deferred no-op.
void initialize_debug_variable_system();

// 0x00ff80a0 (via the thunk at 0x005dc9b0... actually reached directly
// as thunk_FUN_00ff80a0 from WinMain, right before the engine bootstrap
// FUN_005dcdf0). A second, distinct engine-subsystem bootstrap
// ("RuntimeInit::GlobalTempBuffer"/"RealGraphSystem"/
// "RealInputSystem"/"Locale"/"FMV", each a named heap allocation) on
// top of the one already reconstructed in EngineBootstrap.cpp. Reuses
// the same one-shot service_holder lazy-init inline (reproduced here by
// calling hp6::platform::service_holder() for real, rather than
// duplicating its body, same as every other site in this project that
// inlines the identical block). The five named subsystem allocations
// and everything that follows them (input system, locale tables, FMV
// video-playback object) are not required for this project's own
// render/chapter-flow/message-loop path and are a documented deferred
// no-op.
void initialize_runtime_services();

// 0x00ffa0d0. Called once from WinMain, immediately after the startup
// SystemParametersInfoA save, with the command line (param_3) as an
// implicit "this" (EDI). Parses "-key=value" tokens out of the command
// line into a small table (up to 0x20 entries) on the same object. No
// test in this project passes command-line overrides, and no traced
// caller reads the resulting table back yet, so this is a documented
// deferred no-op; the real per-token parse (isspace-delimited,
// '='-split, FUN_005e0630 to duplicate the value) is left unreconstructed.
void parse_command_line_overrides(const char* command_line);

// 0x006882b0. Called once from WinMain, right after run_message_loop
// returns and the dynamic render resources are torn down
// (hp6::render::shutdown_render_device). Its first block is guarded by
// DAT_00cf1b0c, which only becomes non-zero when WinMain's own
// bootup-service object (DAT_00cef698) is non-null — itself a
// documented deferred nullptr in WinMain.cpp — so that block is
// honestly unreachable here. The four unconditional trailing calls
// (FUN_006dad2c, FUN_006b281f, FUN_006d9db4, FUN_006d93bb) are not
// otherwise traced; documented deferred no-op.
void release_cached_debug_strings();

// 0x00ff8ce0. The hardware/software compatibility gate WinMain checks
// right after the engine bootstrap (FUN_005dcdf0): when DAT_00cf1b00 is
// zero it returns true immediately. hp6::platform::
// initialize_file_system_storage (0x00688130), WinMain's own direct
// caller just before this gate, unconditionally clears DAT_00cf1b00 to
// 0 and nothing between the two calls writes it again — so the gate's
// own inner compatibility check (FUN_006883a0/FUN_006884b0/
// FUN_00688460/FUN_00688400, presumably a DirectX runtime version
// check) is confirmed unreachable in this call chain and is a
// documented abort()-ing placeholder, matching this project's
// established doctrine for confirmed-dead branches.
bool is_platform_compatible();

} // namespace hp6::platform
