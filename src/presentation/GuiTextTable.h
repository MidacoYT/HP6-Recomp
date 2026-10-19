#pragma once
#include <cstdint>

namespace hp6::presentation {

// 0x00429430 (stdcall). The shared key/string map at a fixed address
// (0x00cf8168, referenced by 45 different functions across the binary
// per this project's own cross-referencing) that ScreenLanguageSelectTask
// ::Update (and presumably other screens) populate with localized UI
// text, later read by a separate GUI/text renderer
// (PluginPresentationBitmap_BuildRenderQuad, 0x0056baa0 -- not yet
// integrated, see architecture/startup_path.md).
//
// Confirmed real contract: a no-op when either `key` or `text` is null
// (the exported x86's own `if (param_2 == 0 || param_3 == 0) return;`
// guard); otherwise inserts `text` under `key`, replacing any existing
// value. The exported x86 implements this with its own hand-rolled,
// SEH-wrapped hybrid linked-list/hash structure (several more thunked
// allocator calls, not traced) -- this reconstruction preserves the
// observable insert/update contract faithfully without claiming to
// replicate that internal node layout byte-for-byte, since nothing else
// integrated in this project yet depends on it.
void update_localized_text(std::uint32_t table, const char* key, const wchar_t* text);

// Test accessor, not present in the exported binary: reads back the
// value update_localized_text stored under `key`, or null when no
// entry exists. Same justification as TaskSystem's own introspection
// helpers — the reconstruction keeps the insert/update contract, not
// the binary's internal node layout, so tests observe through here.
const wchar_t* read_localized_text(std::uint32_t table, const char* key);

// Test accessor, also not present in the exported binary: drops every
// entry (and every table) this reconstruction has accumulated, so a
// test can start from the game's freshly-booted empty state.
void clear_localized_text_tables();

} // namespace hp6::presentation
