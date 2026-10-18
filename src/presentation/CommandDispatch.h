#pragma once
#include <cstddef>
#include <cstdint>

namespace hp6::presentation {

using CommandHandler = std::int32_t (*)(const char* args);

struct CommandTableEntry {
    const char* name;
    CommandHandler handler;
};

// 0x0048abf0 (stdcall; `verb` is received through EDI in the exported
// x86, not a stack argument). Looks `verb` up by exact-match comparison
// (a custom byte-for-byte loop in the exported x86; std::strcmp here is
// behaviourally equivalent for the equality test this function only
// ever uses) against the 93-entry presentation/storage command table at
// 0x00876ab0 — an array of pointers to {name, handler} pairs, preserved
// as architecture/presentation_commands.csv — and calls the matching
// handler with `args`. Returns the handler's result, or 0 when no
// entry matches (the exported x86 returns the unused comparison result
// with its low byte masked to 0; only that low byte is ever
// meaningful, so a plain 0 is equivalent here).
std::int32_t find_and_invoke_command(const char* verb, const char* args);

// 0x0048ab90 (stdcall; `command` is received through ESI, not a stack
// argument). Splits `command` at its first space into a verb and the
// remaining argument text (no space: the whole string is the verb, no
// arguments), then dispatches through find_and_invoke_command,
// returning its result. The exported x86's own decompile shows this
// as void, but at least one real caller
// (0x00429de0, part of the ScreenAutosaveWarningMessage screen task —
// see architecture/startup_path.md) reads AL immediately after calling
// it and branches on it, which only makes sense if the leftover EAX
// from find_and_invoke_command's tail call is the intended result;
// returning it explicitly here is equivalent and makes that caller
// reconstructable. The exported x86 copies into a fixed 256-byte stack
// buffer with no bounds check; this reconstruction truncates instead
// of overflowing for any command longer than that.
std::int32_t dispatch_presentation_command(const char* command);

// Each entry is itself a small task-like object: field+0 is its own
// vtable (slot 0 is its "activate" method), field+4 is its name.
struct ScreenTaskEntry {
    void* vtable;
    const char* name;
};

// 0x0048ce30 ("HybridScreenActivateMappedTask"; cdecl, `name` explicit).
// Looks `name` up by exact-match comparison (same custom byte loop as
// find_and_invoke_command, reconstructed the same way) against the
// 26-entry screen-task table at 0x00870948 — an array of pointers to
// pre-built per-screen task objects (ScreenMainMenu,
// ScreenAutosaveWarningMessage, ScreenAttractMode, ScreenFMV, ...; see
// architecture/startup_path.md) — and, on a match, calls that entry's
// own vtable slot 0 (confirmed thiscall from the exported x86: ECX
// stays the entry pointer itself through the vtable fetch and is never
// pushed) with no explicit arguments. This is the resolver behind
// `Presentation.Screens.ActivateScreenWithFocus <name>` and its
// siblings. The exported x86 returns a fixed 1 in its low byte whether
// or not a match was found — not a success/failure signal here.
std::int32_t activate_mapped_screen_task(const char* name);

} // namespace hp6::presentation
