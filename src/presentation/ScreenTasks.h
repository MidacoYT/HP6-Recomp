#pragma once
#include <cstdint>

namespace hp6::presentation {

// 0x0050b920 ("ScreenTask_ScreenMainMenu_Enqueue"; thiscall). One of
// 24 near-identical per-screen task constructors (see
// architecture/presentation_commands.csv and
// architecture/startup_path.md for the full list — ScreenTitlePage,
// ScreenAutosaveWarningMessage, ScreenAttractMode, ScreenFMV, ...),
// reached by name through activate_mapped_screen_task
// (src/presentation/CommandDispatch.h) when a
// "Presentation.Screens.ActivateScreenWithFocus ScreenMainMenu"-style
// command resolves. Allocates a 16-byte task object (vtable, a byte
// ready flag, a secondary-interface pointer used only by its
// destructor's multiple-inheritance thunk, and an int32 state —
// zero-initialized by the allocator), submits it to
// PresentationTaskHandler (submit_task), and stores the result at
// `self + 8` (the caller's own object; its exact type is not
// resolved, kept as a raw offset per house style).
void enqueue_main_menu_screen_task(void* self);

// The ScreenMainMenu task's own "Update" (vtable slot 3, thiscall, no
// explicit arguments beyond `this`). A 2-step gate, not yet reached by
// a real caller here (nothing in this project calls through the
// task's vtable yet) but reconstructed for completeness and testing:
// at state 1, waits for `*dialog_active == 0` (no modal dialog — see
// 0x005d3070's "GUI_CONFIRMSAVE" prompt) and
// `*dialog_response == 1` (user confirmed); dispatches
// "Presentation.Screens.ActivateScreenWithFocus ScreenTitlePage" and
// advances to state 2. `*dialog_response == 0` (user declined) resets
// to state 0. Any other value (including the exported x86's -1
// "unanswered" sentinel) leaves the state unchanged. A no-op at every
// other state.
void update_main_menu_screen_task(void* task);

// 0x0050ba70 ("ScreenTask_ScreenTitlePage_Enqueue"; thiscall). Same
// shape as enqueue_main_menu_screen_task, but this task object is 24
// bytes (it carries two extra fields the ScreenMainMenu task doesn't:
// a byte pair at +0xc and an idle-timer sub-task pointer at +0x14 —
// see below). This is the real confirmed entry point for
// `ScreenTitlePage`, the actual first visible screen (see
// architecture/startup_path.md).
//
// Only the constructor, the shared destructor, and "IsReady" (vtable
// slot 1) are reconstructed. "IsReady" has a real, important side
// effect — constructing and submitting a
// "Presentation:TaskHandler::Task_IdleInputTimer" sub-task (see
// enqueue_idle_input_timer_task below), which is how the
// inactivity-driven AttractMode switch (traced two sessions ago) gets
// armed. Vtable slots 2 ("is self enqueued in some list, by a field
// at self+0x14") and 3 ("Update", a large >256-byte-buffer function)
// are NOT reconstructed — genuinely out of scope here — but nothing in
// this project calls through them, so their absence doesn't affect
// what is implemented.
// enqueue_title_page_screen_task's IsReady (0x00451ec0) also
// constructs and submits a second, nested task object —
// "Presentation:TaskHandler::Task_IdleInputTimer" (vtable at
// 0x00821508: destructor 0x00429c70, IsReady 0x0059f450, Update
// 0x0059f460) — fully reconstructed as a private implementation
// detail in ScreenTasks.cpp (it is not independently constructed
// anywhere else, so it has no separate public entry point here). A
// tiny, self-contained idle-detection timer: its field+8 caches the
// tick (DAT_00d644f8) at the last detected input, and its Update
// re-checks for input (0x004ccca0, not reconstructed — an external
// dependency) whenever field+0xc is 0 or the -1 "not yet initialized"
// sentinel, refreshing field+8 on a hit. This is the mechanism behind
// the inactivity timeout that eventually triggers
// `Presentation.Screens.ActivateScreenWithFocus ScreenAttractMode`
// (not confirmed by a direct call site, but consistent with
// 0x0048d2a0's AttractMode-selection logic traced earlier).
void enqueue_title_page_screen_task(void* self);

// 0x0050bae0 ("ScreenTask_ScreenAutosaveWarningMessage_Enqueue";
// thiscall). A simpler, fully-reconstructed 12-byte task (vtable,
// ready flag, one int32 field: a tick/3 snapshot) — no secondary
// interface, no dedicated "current task" global, unlike ScreenMainMenu/
// ScreenTitlePage.
//
// Its "Update" (vtable slot 3, 0x00429de0) is the confirmed mechanism
// for what happens after the save-warning prompt: once >=2000 (tick/3)
// units have elapsed since construction, it activates
// "ScreenAutosaveWarningMessagePrompt" (without focus) if not already
// active; if no input has been detected by then, it returns (waiting
// for a response) and, once >=8000 units have elapsed with still no
// input, it dispatches
// "Presentation.Screens.ActivateScreenWithFocus ScreenTitlePageCut_PC"
// — i.e. the title-page cutscene plays automatically if the player
// never presses anything. Vtable slot 2 (0x00429dd0, not otherwise
// called from this project) deactivates the same prompt screen.
void enqueue_autosave_warning_screen_task(void* self);

// The task's own "Update" (vtable slot 3, thiscall, no explicit
// arguments). See enqueue_autosave_warning_screen_task's doc comment
// for the full behaviour. Returns 1 once it has either activated the
// prompt while input was already seen, or dispatched the
// ScreenTitlePageCut_PC cutscene; 0 while still waiting.
std::int32_t update_autosave_warning_screen_task(void* task);

// 0x0050bcd0 ("ScreenTask_ScreenFMV_Enqueue"; thiscall). A 44-byte
// task — the largest seen so far. Only the constructor and shared
// destructor are reconstructed here. Its vtable slots 1-3 are
// substantially more complex than every other screen task traced in
// this project (slot 2, 0x0053df20, indexes the 16-entry AttractMode
// FMV table at 0x008923e8 via DAT_00cef5b8 — the same table read by
// 0x00fec210/FMVPlayback, see architecture/startup_path.md; slot 3,
// 0x0053e000, is a 12-state jump-table dispatcher with a 0x170-byte
// local buffer) — genuinely out of scope here. This confirms ScreenFMV
// is the real screen behind video playback, but its own state machine
// (how a video is opened/decoded/played/torn down) is not
// reconstructed.
void enqueue_fmv_screen_task(void* self);

// 0x0050bc60 ("ScreenTask_ScreenAttractMode_Enqueue"; thiscall). A
// 48-byte task. Only the constructor and shared destructor are
// reconstructed. Its vtable slot 1 ("IsReady", 0x0048d2a0) is
// confirmed to be the same large `DAT_00cef7b0`-driven state machine
// partially traced several sessions ago (its "case 7" branch emits
// "QuitGame ScreenMainMenu" — see architecture/decisions.md) — this
// resolves what object that state machine belongs to (AttractMode's
// own task), but the machine itself remains out of scope. Slots 2
// (0x0048d3c0) and 3 ("Update", 0x0048d4a0, a 12-state jump-table
// dispatcher with a 0x48-byte local buffer) are likewise not
// reconstructed.
void enqueue_attract_mode_screen_task(void* self);

// 0x0050b8b0 ("ScreenTask_ScreenLanguageSelect_Enqueue"; thiscall). A
// 24-byte task. The constructor, shared destructor, "IsReady" (vtable
// slot 1, 0x00429850), and slot 2 (0x00429860) are reconstructed;
// vtable slot 3 ("Update", 0x00429890) is a large (0x118-byte local
// buffer) language-list/navigation state machine, out of scope here.
// The task's real primary vtable lives at 0x00821528 (read live from
// the Ghidra project: slot 0 = dtor 0x00429810, slot 1 = 0x00429850,
// slot 2 = 0x00429860, slot 3 = Update 0x00429890 as above, slot 4/
// +0x10 = 0x00405a30). That slot 4 is the "Render" entry
// PresentationTaskHandler's own message handler calls on every active
// task when it receives the named message "iMsgDoRender" (see
// architecture/startup_path.md, "iMsgDoRender / GUI rendering trail"
// — PresentationTaskHandler's real vtable at 0x0083d194, slot+0xc is
// its OnMessage, which for that message walks its task list and calls
// each task's own vtable slot+0x10) — and for ScreenLanguageSelect
// specifically it is **confirmed empty** (`FUN_00405a30` is a bare
// `return;`, no body at all). So this task does not draw its own menu
// text directly; Update's real body instead writes the localized
// strings ("GUI_LANGUAGEMENU%d", 0-15) into a shared key/string map
// object at a fixed address (0x00cf8168, via FUN_00429430, itself
// confirmed to be a plain std::map-style insert/update — not a draw
// call) that a separate, not-yet-located central GUI/text renderer
// must read from. That renderer, and whatever actually calls
// FUN_005f4b50-or-equivalent to fire "iMsgDoRender" once per frame,
// remain unresolved as of 18 oct 2026 — deliberately left unresolved
// rather than guessed at; see architecture/startup_path.md for the
// full trail of what was checked and ruled out.
void enqueue_language_select_screen_task(void* self);

// ScreenLanguageSelectTask's vtable slot 1 ("IsReady", 0x00429850).
// Neither 0x00429850 nor the slot-2 address below have their own
// Ghidra function object in this project's analysis (they sit in an
// unclaimed gap between FUN_00429780 and FUN_00429890, likely only
// ever reached through the vtable, so auto-analysis never carved them
// out) — read directly via live disassembly instead of a decompiled
// export: `MOV dword ptr [ECX+0x14],0x0; MOV AL,0x1; RET`.
// Unconditionally resets the Update state machine (see above) to its
// initial state (0) and reports "ready" — the same unconditional-reset
// shape as every sibling task's IsReady in this file.
std::int32_t query_ready_language_select_screen_task(void* task);

// ScreenLanguageSelectTask's vtable slot 2 (0x00429860), also read
// directly via live disassembly. Update's state-1 branch (not
// reconstructed) submits a "Presentation:TaskHandler::Task_
// IdleInputTimer" sub-task (the same class enqueue_title_page_
// screen_task's IsReady already constructs, see above) and stores the
// result at this task's own +0xc. This slot walks
// PresentationTaskHandler's own task list (+0xc, a singly-linked list
// of {next, ..., tag} nodes tagged at +8) looking for a node whose tag
// equals that pending sub-task pointer, and if found, sets the
// sub-task's own ready_flag (its +0x4 byte — every task here shares
// that layout) to 1. In other words: once the pending idle-timer
// sub-task is confirmed registered with the handler, flag it ready.
void mark_language_select_pending_task_ready(void* task);

} // namespace hp6::presentation
