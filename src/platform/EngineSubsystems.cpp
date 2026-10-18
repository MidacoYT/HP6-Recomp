#include "platform/EngineSubsystems.h"
#include "platform/HandleRegistry.h"

#include <cstdlib>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern std::uint8_t DAT_00892df0;  // reset_particle_system_tables's one-shot guard
extern std::int32_t DAT_00cf1b00;  // the compatibility gate's own fast-path flag

namespace hp6::platform {

void reset_particle_system_tables() {
    if (DAT_00892df0 != 0) {
        return;
    }
    DAT_00892df0 = 1;

    // NOT YET INTEGRATED: the four partial zero-fills over
    // [0x00851450, 0x008516ad) and FUN_006b39e2(2,1)/FUN_006b3950(0,1),
    // a documented deferred no-op (see EngineSubsystems.h).
}

void initialize_debug_variable_system() {
    // NOT YET INTEGRATED: see EngineSubsystems.h. Documented deferred
    // no-op; a pure developer-tooling subsystem nothing else reads.
}

void initialize_runtime_services() {
    service_holder();

    // NOT YET INTEGRATED: the five named "RuntimeInit::*" subsystem
    // allocations (GlobalTempBuffer, RealGraphSystem, RealInputSystem,
    // Locale, FMV) and everything that follows them, a documented
    // deferred no-op (see EngineSubsystems.h).
}

void parse_command_line_overrides(const char* command_line) {
    (void)command_line;
    // NOT YET INTEGRATED: see EngineSubsystems.h. Documented deferred
    // no-op.
}

void release_cached_debug_strings() {
    // The DAT_00cf1b0c-guarded block (FUN_006b1c22/FUN_006b1f4b/
    // FUN_006dadcf) is confirmed unreachable here — see
    // EngineSubsystems.h. The four unconditional trailing calls
    // (FUN_006dad2c/FUN_006b281f/FUN_006d9db4/FUN_006d93bb) are a
    // documented deferred no-op.
}

bool is_platform_compatible() {
    if (DAT_00cf1b00 == 0) {
        return true;
    }
    // NOT YET INTEGRATED: confirmed unreachable in this project's own
    // WinMain call chain (see EngineSubsystems.h) — aborting here
    // matches the project's doctrine for confirmed-dead branches
    // rather than fabricating the real DirectX-version-style check.
    std::abort();
}

} // namespace hp6::platform
