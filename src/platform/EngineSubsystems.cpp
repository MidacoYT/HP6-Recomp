#include "platform/EngineSubsystems.h"
#include "platform/HandleRegistry.h"

#include <cctype>
#include <cstdlib>
#include <cstring>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern std::uint8_t DAT_00892df0;  // reset_particle_system_tables's one-shot guard
extern std::int32_t DAT_00cf1b00;  // the compatibility gate's own fast-path flag

// parse_command_line_overrides's own fixed global storage, confirmed via
// live disassembly of its real call site (WinMain, 0x005d93b3-0x005d93eb):
// WinMain truncating-copies its own command line into two adjacent
// 0x200-byte buffers before calling this function with EDI pointing at
// the first one. The override tables this function fills live at fixed
// offsets from that SAME base address, in the same static data region.
extern char DAT_00d63f68[0x200];  // this function's own copy of the command line (the one it scans)
extern char DAT_00d64168[0x200];  // a second copy the real call site also makes; not read by this function
extern char* DAT_00d64368[0x20];  // "-flag" plain overrides (base+0x400)
extern char* DAT_00d643e8[0x20];  // "-key=value" keys (base+0x480)
extern char* DAT_00d64468[0x20];  // "-key=value" values (base+0x500)
extern std::int32_t DAT_00d644e8; // plain-override count (base+0x580)
extern std::int32_t DAT_00d644ec; // pair-override count (base+0x584)

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

namespace {

// 0x005e0630 (fastcall; `cursor` is the implicit ESI register in the
// exported x86 -- confirmed via live disassembly of both call sites in
// parse_command_line_overrides below, which each pass the address of
// their own local cursor variable). Extracts the next token starting at
// *cursor: a quote-delimited run (embedded whitespace allowed, opening
// and closing quote stripped) when the token starts with `"` or `'`,
// else a run up to the next whitespace. Null-terminates the token in
// place and advances *cursor past it, returning the token's start.
char* extract_next_token(char** cursor) {
    char* start = *cursor;
    const char quote = *start;
    if (quote == '"' || quote == '\'') {
        ++start;
        *cursor = start;
        char* scan = start;
        while (*scan != '\0' && *scan != quote) {
            ++scan;
            *cursor = scan;
        }
    } else {
        while (**cursor != '\0' && std::isspace(static_cast<unsigned char>(**cursor)) == 0) {
            ++*cursor;
        }
    }
    char* end = *cursor;
    if (*end != '\0') {
        *end = '\0';
        *cursor = end + 1;
    }
    return start;
}

} // namespace

// 0x00ffa0d0; see EngineSubsystems.h. Tokenizes the command line into
// "-flag" plain overrides and "-key=value" pairs, confirmed by live
// disassembly of the exported x86 (the decompiled C alone hides which
// register FUN_005e0630's own calls operate on).
void parse_command_line_overrides(const char* command_line) {
    // The real call site truncating-copies the raw command line into two
    // fixed buffers before calling this function; reproduced here even
    // though only the first copy is ever read below, for fidelity to
    // what the exported x86 actually does.
    std::strncpy(DAT_00d63f68, command_line, sizeof(DAT_00d63f68) - 1);
    DAT_00d63f68[sizeof(DAT_00d63f68) - 1] = '\0';
    std::strncpy(DAT_00d64168, command_line, sizeof(DAT_00d64168) - 1);
    DAT_00d64168[sizeof(DAT_00d64168) - 1] = '\0';

    char* cursor = DAT_00d63f68;
    for (;;) {
        while (*cursor != '\0' && std::isspace(static_cast<unsigned char>(*cursor)) != 0) {
            ++cursor;
        }

        if (*cursor == '-') {
            ++cursor;
            if (*cursor == '\0') {
                return;
            }
            char* const key = cursor;
            char* value = nullptr;
            while (std::isspace(static_cast<unsigned char>(*cursor)) == 0 &&
                   *cursor != '=' && *cursor != '\0') {
                ++cursor;
            }
            if (*cursor == '=') {
                value = cursor + 1;
            }
            if (*cursor != '\0') {
                *cursor = '\0';
                ++cursor;
            }
            if (value != nullptr) {
                // Real quirk, reproduced faithfully: this call both
                // extracts the value token AND advances `cursor` past
                // it, synchronizing the outer scan with whatever
                // extract_next_token's own quote-handling consumed.
                value = extract_next_token(&cursor);
            }
            if (DAT_00d644ec < 0x20) {
                DAT_00d643e8[DAT_00d644ec] = key;
                DAT_00d64468[DAT_00d644ec] = value;
                ++DAT_00d644ec;
            }
        } else {
            if (*cursor == '\0') {
                return;
            }
            // Real quirk, reproduced faithfully: when this table is
            // full, the exported x86 skips extract_next_token entirely
            // rather than still advancing past the token -- unlike the
            // "-flag"/"-key=value" branch above, which always consumes
            // its token even when ITS table is full.
            if (DAT_00d644e8 < 0x20) {
                char* token = extract_next_token(&cursor);
                DAT_00d64368[DAT_00d644e8] = token;
                ++DAT_00d644e8;
            }
        }

        if (*cursor == '\0') {
            return;
        }
    }
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
