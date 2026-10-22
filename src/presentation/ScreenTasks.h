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
// 24-byte task. All of its task-object vtable slots are reconstructed:
// the constructor and shared destructor here, "IsReady" (vtable slot 1,
// 0x00429850) and slot 2 (0x00429860) as standalone entry points below,
// and "Update" (vtable slot 3, 0x00429890) as
// update_language_select_screen_task below. The task's real primary vtable lives at 0x00821528 (read live from
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
// text directly; Update's body writes the localized strings
// ("GUI_LANGUAGEMENU%d", 0-15) into a shared key/string map object at
// a fixed address (0x00cf8168 — FUN_00429430, reconstructed as
// update_localized_text, GuiTextTable.h, and called for real by the
// now-reconstructed Update, see update_language_select_screen_task)
// that a separate, not-yet-located central GUI/text renderer
// must read from. That renderer, and whatever actually calls
// FUN_005f4b50-or-equivalent to fire "iMsgDoRender" once per frame,
// remain unresolved as of 19 oct 2026 — deliberately left unresolved
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

// ScreenLanguageSelectTask's own "Update" (vtable slot 3, 0x00429890;
// thiscall, no explicit arguments beyond `this`), reconstructed on
// 19 oct 2026. A 3-state machine driven per frame through the same
// vtable dispatch as every other screen task's Update:
//
// State 0 — chapter-load gate: waits until DAT_00cef610 points at an
// object whose +0xb0 byte is 0 (the same startup-context pointer
// ChapterLoadStateMachine's state machine reads; while its own load is
// pending the byte is non-zero and this task stays in state 0), then
// advances to state 1.
//
// State 1 — arm + fill the menu text: snapshots DAT_00d644f8 (the
// frame tick) at task+0x10; constructs and submits the same 0x10-byte
// "Presentation:TaskHandler::Task_IdleInputTimer" sub-task
// enqueue_title_page_screen_task's IsReady constructs (ready_flag=0,
// the shared 0x00821508 vtable, and the sub-task's own +0xc state set
// to the -1 "not yet armed" sentinel), storing the submission result
// at task+0xc so slot 2 above can flag it ready when it shows up in
// the handler's own list; then performs the state's surviving
// observable: 16 writes into the shared text table 0x00cf8168
// (update_localized_text, GuiTextTable.h), one per key
// "GUI_LANGUAGEMENU0".."GUI_LANGUAGEMENU15", each either the wide
// display text 0x005e5e40 finds for that language (deferred test
// callback in this reconstruction) in the language service object
// DAT_00fcae04 (narrow char* entries in the vector at +0xb8/+0xbc;
// the service's own +0x04 name buffer is what the lookup is keyed
// against), or the wide " " constant at 0x0081fca8 for slots beyond
// the installed language count. The exported x86 follows the fill
// with a widget-registry resync (thunk_FUN_0100e910's current-language
// row, FUN_00435110's find-by-name, FUN_00416f70) whose only effect is
// writing each matching UI widget's +0xc8 field
// with that language's index — deferred as documented in
// ScreenTasks.cpp because none of those widgets exists in this
// project yet. ADVANCES STRAIGHT TO STATE 2.
//
// State 2 — idle + commit: on entry, when exactly one language is
// installed (the vector's byte distance is 4) or the service's own
// +0x98 marker buffer contains "WBSUB", resets the selected-language
// index DAT_00879654 to 0; then raises DAT_00d6b8a0's bit 0 and
// zeroes DAT_00d6b898/DAT_00d6e5b8. A 15000-tick idle-selection scan
// that re-derives DAT_00879654 from the current-process language
// row (thunk_FUN_0100e910/FUN_005e5f30/FUN_005e5f00 — the IdleInput-
// Timer sub-task's own +0x8 tick is the idle reference) is deferred
// as documented in ScreenTasks.cpp. Once more than 0x14d (333)
// tick/3 units have elapsed since the state-1 snapshot, commits the
// selection through the confirm path (FUN_00fd7920's reconstruction;
// integrated in ScreenTasks.cpp): copy the selected language name
// (0x005e5de0) into the service's +0x04 buffer, raise the registry
// DAT_00d46aa8's +0xa0 flag bit 2 unless the name is one of the two
// built-in English markers ("en-GB" at 0x0083e694 / "en-US" at
// 0x0083e68c, the FUN_005da670 window-title update and the localized
// .str hook thunk_FUN_00ff7f20 being deferred), set the resolved
// chapter index DAT_00d472dc to 8 and re-run
// resolve_and_load_chapter(DAT_00fcadec, "FrontEnd") — the real,
// already-integrated chapter hop out of the language menu — and then
// return 1 (task done).
//
// The exported x86's EAX at each fall-through exit carries whatever
// register the last arithmetic left there (DAT_00cef610 for state 0,
// `this` for state 1, the elapsed tick/3 value for state 2's
// not-yet-committed exit, `state - 2` for states beyond 2); the low
// byte is the only part any caller could meaningfully observe, and
// those fall-through values are replicated literally in
// ScreenTasks.cpp the way ChapterLoadStateMachine.cpp already does
// for its own `& 0xffffff00` returns.
std::int32_t update_language_select_screen_task(void* task);

// 0x005da610 and 0x0048c8f0. The LanguageSelection command decides
// whether the installed SKU needs the language menu, then loads the
// matching chapter through resolve_and_load_chapter.
bool is_transgaming_macos();
std::int32_t handle_language_selection_command(const char* args);
std::int32_t begin_language_selection(); // 0x00505670

} // namespace hp6::presentation
