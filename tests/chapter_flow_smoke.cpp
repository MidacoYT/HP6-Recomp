// Local behavior checks for FUN_00ff1430, called from
// ChapterLoadStateMachine_Update's final state once a chapter finishes
// loading (see architecture/startup_path.md). Game-space symbols get
// test-local definitions, the way the other smoke tests already do.
#include "presentation/ChapterFlow.h"
#include "presentation/CommandDispatch.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

// ---- ChapterFlow.cpp's own extern dependencies ----------------------------
std::uint32_t DAT_00fd1b14 = 0;
void* DAT_00fcd530 = nullptr;
std::uint8_t* DAT_00fcd534 = nullptr;
std::int32_t DAT_00d472dc = 0;
std::uint8_t DAT_0087e014 = 0;
std::uint32_t DAT_00fd178c = 0;
void* DAT_00fd1718 = nullptr;
void* DAT_00fcadec = nullptr;

struct ChapterTableEntry {
    std::int32_t index;
    const char* name;
    std::int32_t next_hint;
};
// Mirrors the real table read live at 0x00892300 (see
// architecture/startup_path.md): LanguageSelect(9), FrontEnd(8),
// BurrowIntro(1), Hogwarts(2), BurrowXmas(3), Hogwarts(4), TheCave(5),
// Hogwarts(6), Hogwarts(7), Hogwarts(10).
ChapterTableEntry DAT_00892300[10] = {
    {9, "LanguageSelect", 0}, {8, "FrontEnd", 0},       {1, "BurrowIntro", 2},
    {2, "Hogwarts", 3},       {3, "BurrowXmas", 4},     {4, "Hogwarts", 5},
    {5, "TheCave", 6},        {6, "Hogwarts", 7},       {7, "Hogwarts", 8},
    {10, "Hogwarts", 0},
};
// Mirrors the real 11-entry table at 0x0080a91c.
const char* PTR_DAT_0080a91c[11] = {
    "NONE",        "BURROW_INTRO", "HOGWARTS_1",    "BURROW_XMAS", "HOGWARTS_2",
    "THE_CAVE",    "HOGWARTS_3",   "ENDLESS_DAY",   "FRONT_END",   "LANGUAGE_SELECT",
    "POTIONS_GAME",
};

// ---- hp6::platform::service_holder dependencies --------------------------
void* PTR_FUN_0083e55c[27] = {};
void* DAT_00fcdd74[6] = {};
void** DAT_00cef6bc = nullptr;
std::uint32_t DAT_00fd1254 = 0;
void cleanup_00711e60() {}
void* PTR_FUN_007159b0 = nullptr;
extern void* PTR_FUN_00715d0c[1] = {};
extern void* PTR_LAB_00715da8[26] = {};
void* PTR_LAB_0071b520 = nullptr;
std::uint32_t DAT_008998c8 = 0;

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

std::int32_t unused_command_handler(const char*) { return 0; }

int deactivate_all_calls = 0;
std::int32_t deactivate_all_handler(const char*) {
    ++deactivate_all_calls;
    return 0;
}
int front_end_calls = 0;
std::int32_t front_end_handler(const char*) {
    ++front_end_calls;
    return 0;
}
int activate_with_focus_calls = 0;
char last_activate_with_focus_args[64] = {};
std::int32_t activate_with_focus_handler(const char* args) {
    ++activate_with_focus_calls;
    std::strncpy(last_activate_with_focus_args, args ? args : "",
                 sizeof(last_activate_with_focus_args) - 1);
    return 0;
}

} // namespace

// Repetition macros rather than a hand-counted literal list (see
// tests/screen_tasks_smoke.cpp's own note on why: an earlier version of
// that file miscounted and silently left slots null).
#define HP6_X5(x) x, x, x, x, x
#define HP6_X10(x) HP6_X5(x), HP6_X5(x)

using hp6::presentation::CommandTableEntry;
const CommandTableEntry kUnusedCommand{"", &unused_command_handler};
const CommandTableEntry kDeactivateAll{"Presentation.Screens.DeactivateAllScreensWithFocus",
                                       &deactivate_all_handler};
const CommandTableEntry kFrontEnd{"OnActivateFrontEndChapter", &front_end_handler};
const CommandTableEntry kActivateWithFocus{"Presentation.Screens.ActivateScreenWithFocus",
                                           &activate_with_focus_handler};
extern const CommandTableEntry* const PTR_PTR_00876ab0[93] = {
    &kDeactivateAll, &kFrontEnd, &kActivateWithFocus,
    HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand),
    HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand),
    HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand),
    &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand,
    &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand,
};

using hp6::presentation::ScreenTaskEntry;
void* unused_screen_vtable[1] = {};
const ScreenTaskEntry kUnusedScreen{unused_screen_vtable, ""};
extern const ScreenTaskEntry* const PTR_PTR_00870948[26] = {
    HP6_X10(&kUnusedScreen), HP6_X10(&kUnusedScreen), HP6_X5(&kUnusedScreen), &kUnusedScreen,
};

int main() {
    using namespace hp6::presentation;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::uint8_t chapter_manager[0x90] = {};
    std::uint8_t screen_manager_adjacent[0x2c0] = {};
    DAT_00fcd534 = screen_manager_adjacent;

    // ---- chapter 9: LANGUAGE_SELECT -----------------------------------------
    chapter_manager[4] = 0xff;
    chapter_manager[0x8c] = 0xaa;
    DAT_00d472dc = 9;
    deactivate_all_calls = 0;
    activate_with_focus_calls = 0;
    emit_chapter_entry_screen_command(chapter_manager);
    expect("language_select_deactivates_first", deactivate_all_calls == 1);
    expect("language_select_activates", activate_with_focus_calls == 1);
    expect("language_select_args",
           std::strcmp(last_activate_with_focus_args, "ScreenLanguageSelect") == 0);
    expect("language_select_bit_cleared", (chapter_manager[4] & 1) == 0);
    expect("language_select_field_8c_cleared", chapter_manager[0x8c] == 0);

    // ---- chapter 8: FRONT_END ------------------------------------------------
    chapter_manager[4] = 0xff;
    DAT_00d472dc = 8;
    front_end_calls = 0;
    std::memset(screen_manager_adjacent, 0xcd, sizeof(screen_manager_adjacent));
    emit_chapter_entry_screen_command(chapter_manager);
    expect("front_end_command_dispatched", front_end_calls == 1);
    expect("front_end_buffer_cleared",
           screen_manager_adjacent[0xb4] == 0 && screen_manager_adjacent[0xb4 + 0x1ff] == 0);
    expect("front_end_bit_cleared", (chapter_manager[4] & 1) == 0);

    // ---- any other chapter, first time (DAT_0087e014 == 0) ------------------
    chapter_manager[4] = 0xff;
    DAT_00d472dc = 0;
    DAT_0087e014 = 0;
    activate_with_focus_calls = 0;
    emit_chapter_entry_screen_command(chapter_manager);
    expect("main_osd_activated", activate_with_focus_calls == 1);
    expect("main_osd_args", std::strcmp(last_activate_with_focus_args, "ScreenMainOSD") == 0);

    // ---- any other chapter, subsequent times (DAT_0087e014 != 0) ------------
    chapter_manager[4] = 0xff;
    DAT_0087e014 = 1;
    deactivate_all_calls = 0;
    activate_with_focus_calls = 0;
    emit_chapter_entry_screen_command(chapter_manager);
    expect("no_repeat_main_osd",
           deactivate_all_calls == 1 && activate_with_focus_calls == 0);

    // ---- resolve_and_load_chapter (00576f30) --------------------------------
    DAT_00d472dc = -1;
    resolve_and_load_chapter(DAT_00fcadec, "LANGUAGE_SELECT");
    expect("resolve_language_select_by_fixed_name", DAT_00d472dc == 9);

    DAT_00d472dc = -1;
    resolve_and_load_chapter(DAT_00fcadec, "FRONT_END");
    expect("resolve_front_end_by_fixed_name", DAT_00d472dc == 8);

    DAT_00d472dc = -1;
    resolve_and_load_chapter(DAT_00fcadec, "Global");
    expect("resolve_unknown_name_leaves_untouched", DAT_00d472dc == -1);

    DAT_00d472dc = -1;
    resolve_and_load_chapter(DAT_00fcadec, "LanguageSelect"); // matches via the
                                                               // 0x892300-table fallback
    expect("resolve_language_select_by_table_name", DAT_00d472dc == 9);

    // ---- advance_to_next_chapter (00ff1390) ---------------------------------
    DAT_00d472dc = 9; // LanguageSelect
    advance_to_next_chapter();
    expect("advance_from_language_select_to_front_end", DAT_00d472dc == 8);

    DAT_00d472dc = 8; // FrontEnd
    advance_to_next_chapter();
    expect("advance_from_front_end_to_burrow_intro", DAT_00d472dc == 1);

    if (failures != 0) {
        std::printf("chapter_flow_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("chapter_flow_smoke: all checks passed\n");
    return 0;
}
