#pragma once
#include <cstdint>

namespace hp6::presentation {

// 0x0050b370. NOT YET INTEGRATED: the real screen-manager singleton
// constructor (reached through thunk_FUN_00fe7f30). A lazy-init shell
// identical in shape to hp6::platform::service_holder — this file
// reconstructs that shell faithfully but defers the real constructor,
// matching this project's house style for not-yet-traced singletons.
void* get_screen_manager_singleton();

// 0x00ff1430 (this = EDI in the exported x86, made an explicit parameter
// here). Called from ChapterLoadStateMachine_Update's final state (see
// initialize_chapter_load_state_machine's doc comment) once a chapter
// finishes loading. Always deactivates every focused screen first, then
// emits exactly one chapter-entry command depending on the resolved
// chapter index (DAT_00d472dc, written by the chapter-name-resolution
// chain rooted at FUN_00576f30/FUN_0053eb40 — NOT YET INTEGRATED, see
// architecture/startup_path.md for why "Global" does not obviously
// resolve to any fixed entry): chapter 8 (FRONT_END) emits
// "OnActivateFrontEndChapter" and clears a 0x200-byte text buffer on the
// screen manager; chapter 9 (LANGUAGE_SELECT) emits "Presentation.
// Screens.ActivateScreenWithFocus ScreenLanguageSelect" — the command
// this project has been looking for since the start of this session;
// any other chapter, the first time this runs (DAT_0087e014=="not yet
// done"), emits "Presentation.Screens.ActivateScreenWithFocus
// ScreenMainOSD" instead. Always clears bit 0 of chapter_manager+4 and
// chapter_manager+0x8c before returning.
void emit_chapter_entry_screen_command(std::uint8_t* chapter_manager);

// 0x0048f240. Lazy-init shell identical in shape to
// get_screen_manager_singleton/hp6::platform::service_holder; the real
// constructor (thunk_FUN_00fdf6b0) is a documented deferred no-op.
void* get_bootup_flow_singleton();

// 0x00576f30 ("this" = DAT_00fcadec at every real call site found in
// this project so far). Resolves `chapter_name` to a chapter index and
// writes it to DAT_00d472dc: first against the 11-entry fixed table at
// 0x0080a91c (NONE, BURROW_INTRO, HOGWARTS_1, BURROW_XMAS, HOGWARTS_2,
// THE_CAVE, HOGWARTS_3, ENDLESS_DAY, FRONT_END, LANGUAGE_SELECT,
// POTIONS_GAME); on a match there (index != 0), looks up the 10-entry
// {chapter_index, name, next_hint} table at 0x00892300 for the entry
// whose own index matches, and uses that. If `chapter_name` matches
// neither table (the case for the literal "Global" TriggerBootupFlow
// passes — confirmed by reading both tables directly, see
// architecture/startup_path.md), DAT_00d472dc is left untouched and a
// deferred fallback (get_bootup_flow_singleton's slot +0x2c) runs
// instead. On a successful resolution, also clears a feature flag
// (FUN_0056d910, deferred) unless the new chapter is FRONT_END, and
// runs a second deferred hook (thunk_FUN_00fe3520) when the matched
// table entry's third field is non-zero. The function's tail — building
// and dispatching a "ChapterLoading <name>" notification command — is a
// documented deferred no-op block: it only notifies, it does not change
// DAT_00d472dc or any state this project's current tests depend on.
void resolve_and_load_chapter(void* chapter_registry, const char* chapter_name);

// 0x00ff1390 (reached through the thunk at 0x00577390). Advances
// DAT_00d472dc to the NEXT entry's chapter index in the 0x00892300
// table (not the current entry's own "next_hint" field — confirmed by
// re-reading the exported x86: it looks up the entry matching the
// *current* DAT_00d472dc, then reads the index of the table slot right
// after it) and, if that next chapter has a name in the 11-entry fixed
// table, calls resolve_and_load_chapter with it. Table order is
// LanguageSelect(9) -> FrontEnd(8) -> BurrowIntro(1) -> Hogwarts(2) ->
// BurrowXmas(3) -> Hogwarts(4) -> TheCave(5) -> Hogwarts(6) ->
// Hogwarts(7) -> Hogwarts(10) — LanguageSelect is structurally the
// FIRST chapter in this progression table, strong indirect evidence
// (not yet a direct confirmation) that it is meant to be the game's
// actual first screen on a fresh install.
void advance_to_next_chapter();

} // namespace hp6::presentation
