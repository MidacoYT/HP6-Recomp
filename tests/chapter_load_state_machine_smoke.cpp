// Local behavior checks for FUN_004da7b0 ("ChapterLoadStateMachine_Update"
// in the independent IDA knowledge base) — see
// architecture/startup_path.md. Game-space symbols get test-local
// definitions, the way the other smoke tests already do. This test
// targets the state machine's dispatch/transition shape and its one
// critical state (19, 0x13 in the exported x86's own switch) rather than
// a full run from state 1: most intermediate states gate their
// transition on not-yet-reconstructed subsystems (see ChapterFlow.cpp's
// deferred stubs), so driving the whole sequence end-to-end would need
// guessing their real contracts rather than reconstructing them.
#include "presentation/ChapterLoadStateMachine.h"
#include "presentation/CommandDispatch.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

// ---- ChapterLoadStateMachine.cpp's own extern dependencies ---------------
std::uint8_t DAT_00cef5b6 = 0;
std::uint8_t fcadfc_storage[8] = {};
std::uint8_t* DAT_00fcadfc = fcadfc_storage;
std::uint8_t DAT_00d47e68[0x40 * 0x20];
std::int32_t DAT_00fc7e14 = 0;
std::int32_t DAT_00cef560 = 0;
std::int32_t DAT_00d71380 = 0;
std::int32_t DAT_00d71258 = 0;
std::int32_t DAT_00d6dee8 = 0;
std::uint8_t DAT_00cef53f = 0;
void* DAT_00cef7b0 = nullptr;
void* DAT_00fcade0 = nullptr;
std::int32_t DAT_00cef610 = 0;

// ---- ChapterFlow.cpp's own extern dependencies (pulled in transitively
// through emit_chapter_entry_screen_command) -------------------------------
std::uint32_t DAT_00fd1b14 = 0;
void* DAT_00fcd530 = nullptr;
std::uint8_t* DAT_00fcd534 = nullptr;
std::int32_t DAT_00d472dc = 0;
std::uint8_t DAT_0087e014 = 1; // so only one command is ever expected

std::uint32_t DAT_00fd178c = 0;
void* DAT_00fd1718 = nullptr;
void* DAT_00fcadec = nullptr;
struct ChapterTableEntry {
    std::int32_t index;
    const char* name;
    std::int32_t next_hint;
};
ChapterTableEntry DAT_00892300[10] = {
    {9, "LanguageSelect", 0}, {8, "FrontEnd", 0},       {1, "BurrowIntro", 2},
    {2, "Hogwarts", 3},       {3, "BurrowXmas", 4},     {4, "Hogwarts", 5},
    {5, "TheCave", 6},        {6, "Hogwarts", 7},       {7, "Hogwarts", 8},
    {10, "Hogwarts", 0},
};
const char* PTR_DAT_0080a91c[11] = {
    "NONE",        "BURROW_INTRO", "HOGWARTS_1",    "BURROW_XMAS", "HOGWARTS_2",
    "THE_CAVE",    "HOGWARTS_3",   "ENDLESS_DAY",   "FRONT_END",   "LANGUAGE_SELECT",
    "POTIONS_GAME",
};

// ---- hp6::platform::service_holder dependencies --------------------------
void* PTR_FUN_0083e55c = nullptr;
void* PTR_FUN_0083e56c = nullptr;
void* DAT_00fcdd74 = nullptr;
void* DAT_00fcdd78 = nullptr;
void** DAT_00cef6bc = nullptr;
std::uint32_t DAT_00fd1254 = 0;
void cleanup_00711e60() {}
void* PTR_FUN_007159b0 = nullptr;
void* PTR_FUN_00715d0c = nullptr;
void* PTR_LAB_00715da8 = nullptr;
void* PTR_LAB_0071b520 = nullptr;
std::uint32_t DAT_008998c8 = 0;

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

int dispatch_calls = 0;
char last_command[256] = {};
std::int32_t unused_command_handler(const char*) { return 0; }
std::int32_t capturing_handler(const char* args) {
    ++dispatch_calls;
    std::strncpy(last_command, args ? args : "", sizeof(last_command) - 1);
    return 0;
}
} // namespace

#define HP6_X5(x) x, x, x, x, x
#define HP6_X10(x) HP6_X5(x), HP6_X5(x)

using hp6::presentation::CommandTableEntry;
const CommandTableEntry kUnusedCommand{"", &unused_command_handler};
const CommandTableEntry kDeactivateAll{"Presentation.Screens.DeactivateAllScreensWithFocus",
                                       &capturing_handler};
const CommandTableEntry kFrontEnd{"OnActivateFrontEndChapter", &capturing_handler};
const CommandTableEntry kActivateWithFocus{"Presentation.Screens.ActivateScreenWithFocus",
                                           &capturing_handler};
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

    // task layout: +4 = registry pointer, +0xc = byte flag, +0x10 = timer.
    std::uint8_t task[0x14] = {};
    std::uint8_t registry[0x270] = {};
    *reinterpret_cast<std::uint8_t**>(task + 4) = registry;

    // ---- state 0 self-seeds to state 1 --------------------------------------
    *reinterpret_cast<std::int32_t*>(registry + 0x260) = 0;
    *reinterpret_cast<std::int32_t*>(registry + 0x128) = 0; // mode: ready immediately
    update_chapter_load_state_machine(task);
    expect("state0_seeds_and_advances", *reinterpret_cast<std::int32_t*>(registry + 0x260) == 2);
    expect("timer_armed", *reinterpret_cast<std::int32_t*>(task + 0x10) == 0x8c);

    // ---- state 2 always advances to state 3 --------------------------------
    update_chapter_load_state_machine(task);
    expect("state2_advances_to_3", *reinterpret_cast<std::int32_t*>(registry + 0x260) == 3);

    // ---- the critical state: 19 (0x13 in the exported x86's own switch) ---
    // Drives emit_chapter_entry_screen_command and resets the machine to
    // state 0, exactly like FUN_004da7b0's own case 0x12.
    std::memset(registry, 0xcd, sizeof(registry));
    *reinterpret_cast<std::int32_t*>(registry + 0x260) = 19;
    DAT_00d472dc = 9; // LanguageSelect
    DAT_00cef610 = 0;
    dispatch_calls = 0;
    update_chapter_load_state_machine(task);

    expect("critical_state_resets_machine", *reinterpret_cast<std::int32_t*>(registry + 0x260) == 0);
    expect("critical_state_clears_mode", *reinterpret_cast<std::int32_t*>(registry + 0x128) == 0);
    expect("critical_state_clears_byte_124", registry[0x124] == 0);
    expect("critical_state_sets_sentinel_130",
           *reinterpret_cast<std::int32_t*>(registry + 0x130) == 0xff);
    expect("critical_state_sets_sentinel_134",
           *reinterpret_cast<std::int32_t*>(registry + 0x134) == 0xff);
    expect("critical_state_emits_two_commands", dispatch_calls == 2);
    expect("critical_state_emits_language_select",
           std::strcmp(last_command, "ScreenLanguageSelect") == 0);

    if (failures != 0) {
        std::printf("chapter_load_state_machine_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("chapter_load_state_machine_smoke: all checks passed\n");
    return 0;
}
