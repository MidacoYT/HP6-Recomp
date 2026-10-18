#pragma once
#include <cstdint>

namespace hp6::presentation {

// 0x004da7b0 ("ChapterLoadStateMachine_Update" in the independent IDA
// knowledge base at D:\HP\knowledge\hp6). The counterpart to the
// already-mapped Storage.Bootup/Task_Bootup, but for loading the chapter
// resolved by resolve_and_load_chapter (DAT_00d472dc) into the game.
// `task` is the task object (this); `task+4` holds the chapter registry
// pointer (the same object as resolve_and_load_chapter's `chapter_registry`
// parameter, DAT_00fcadec at every real call site seen so far). Dispatches
// on (registry+0x260 - 1), a 20-state machine (states 1..20 in the
// original, cases 0..0x13 here); every state that doesn't fall through to
// the next one also sets the task's timer field (task+0x10) to 0x8c
// (matching the exported x86 exactly). States 0-17 load the resolved
// chapter's actual resources through ~20 not-yet-traced subfunctions
// (deferred below, each a documented no-op placeholder — none of them is
// on a path this project's current tests exercise, since no fixture here
// drives the machine past state 0). **State 18 (case 0x12) is the one
// this project cares about**: it resets the machine back to state 0,
// clears several registry fields, and calls
// hp6::presentation::emit_chapter_entry_screen_command's real trigger
// (thunk_FUN_00ff1430, already integrated) — this is the exact point
// where the resolved chapter (DAT_00d472dc) turns into the
// "ActivateScreenWithFocus ScreenLanguageSelect" (or ScreenMainOSD/
// OnActivateFrontEndChapter) command this session has been tracing since
// its start. Returns the exported x86's own return value (its low byte is
// the only part ever meaningful to any caller found so far).
std::uint32_t update_chapter_load_state_machine(std::uint8_t* task);

} // namespace hp6::presentation
