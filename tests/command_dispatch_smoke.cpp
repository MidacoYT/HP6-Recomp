// Local behavior checks for the presentation command dispatcher
// (0x0048ab90/0x0048abf0), the central "Namespace.Verb Argument"
// interpreter behind every "Presentation.*"/"Storage.*" script command
// (full 93-entry table preserved in
// architecture/presentation_commands.csv; only a few fake entries are
// needed to exercise the dispatch logic itself).
#include "presentation/CommandDispatch.h"

#include <cstdio>
#include <cstring>

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

int echo_calls = 0;
const char* last_echo_args = nullptr;
std::int32_t echo_handler(const char* args) {
    ++echo_calls;
    last_echo_args = args;
    return 42;
}

int noargs_calls = 0;
const char* last_noargs_args = "(not called)";
std::int32_t noargs_handler(const char* args) {
    ++noargs_calls;
    last_noargs_args = args;
    return 0;
}

#ifdef _MSC_VER
#define HP6_STDCALL __stdcall
#else
#define HP6_STDCALL
#endif

int main_menu_activate_calls = 0;
// Thiscall and stdcall are ABI-compatible here for a zero-explicit-arg
// call: both are callee-clean, and the (unused) `this` travels via ECX
// either way, matching tests/pool_list_smoke.cpp's stdcall-declared
// fake for a thiscall-typed vtable slot.
std::int32_t HP6_STDCALL main_menu_activate() {
    ++main_menu_activate_calls;
    return 0;
}
void* main_menu_vtable[1] = {reinterpret_cast<void*>(&main_menu_activate)};

int unused_screen_activate_calls = 0;
std::int32_t HP6_STDCALL unused_screen_activate() {
    ++unused_screen_activate_calls;
    return 0;
}
void* unused_screen_vtable[1] = {reinterpret_cast<void*>(&unused_screen_activate)};

} // namespace

using hp6::presentation::CommandTableEntry;

std::int32_t unused_handler(const char*) { return 0; }
const CommandTableEntry kEcho{"Presentation.Screens.ActivateScreenWithFocus", &echo_handler};
const CommandTableEntry kNoArgs{"TriggerBootupFlow", &noargs_handler};
// The real table has all 93 slots populated (no nulls, since
// find_and_invoke_command dereferences every slot's name on its way to
// a match); the remaining slots here are a filler entry whose name
// never matches this test's commands.
const CommandTableEntry kUnused{"", &unused_handler};

// `extern` is required here: a const variable at namespace scope has
// internal linkage by default in C++ (unlike C), which would otherwise
// leave CommandDispatch.cpp's own `extern` declaration unresolved.
extern const CommandTableEntry* const PTR_PTR_00876ab0[93] = {
    &kEcho, &kNoArgs,
    &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused,
    &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused,
    &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused,
    &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused,
    &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused,
    &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused,
    &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused,
    &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused,
    &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused, &kUnused,
    &kUnused,
};

using hp6::presentation::ScreenTaskEntry;

const ScreenTaskEntry kScreenMainMenu{main_menu_vtable, "ScreenMainMenu"};
const ScreenTaskEntry kUnusedScreen{unused_screen_vtable, ""};

extern const ScreenTaskEntry* const PTR_PTR_00870948[26] = {
    &kScreenMainMenu,
    &kUnusedScreen, &kUnusedScreen, &kUnusedScreen, &kUnusedScreen, &kUnusedScreen,
    &kUnusedScreen, &kUnusedScreen, &kUnusedScreen, &kUnusedScreen, &kUnusedScreen,
    &kUnusedScreen, &kUnusedScreen, &kUnusedScreen, &kUnusedScreen, &kUnusedScreen,
    &kUnusedScreen, &kUnusedScreen, &kUnusedScreen, &kUnusedScreen, &kUnusedScreen,
    &kUnusedScreen, &kUnusedScreen, &kUnusedScreen, &kUnusedScreen, &kUnusedScreen,
};

int main() {
    using namespace hp6::presentation;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // ---- find_and_invoke_command (0048abf0): direct verb/args -------------
    expect("direct_dispatch_found", find_and_invoke_command(
               "Presentation.Screens.ActivateScreenWithFocus", "ScreenMainMenuCameraPos") == 42);
    expect("direct_dispatch_args", echo_calls == 1 &&
                                    last_echo_args != nullptr &&
                                    std::strcmp(last_echo_args, "ScreenMainMenuCameraPos") == 0);
    expect("direct_dispatch_not_found", find_and_invoke_command("NoSuchCommand", nullptr) == 0);

    // ---- dispatch_presentation_command (0048ab90): splits on the first
    // space, with no args when there is none. ------------------------------
    dispatch_presentation_command(
        "Presentation.Screens.ActivateScreenWithFocus ScreenAutosaveWarningMessage");
    expect("split_dispatch_found", echo_calls == 2);
    expect("split_dispatch_args", last_echo_args != nullptr &&
                                   std::strcmp(last_echo_args, "ScreenAutosaveWarningMessage") == 0);

    dispatch_presentation_command("TriggerBootupFlow");
    expect("no_space_dispatch_found", noargs_calls == 1);
    expect("no_space_dispatch_null_args", last_noargs_args == nullptr);

    dispatch_presentation_command("Unknown.Command with args");
    expect("unknown_command_is_a_no_op", echo_calls == 2 && noargs_calls == 1);

    // ---- activate_mapped_screen_task (0048ce30) ----------------------------
    expect("screen_task_found", activate_mapped_screen_task("ScreenMainMenu") == 1);
    expect("screen_task_activated", main_menu_activate_calls == 1);
    expect("screen_task_not_found", activate_mapped_screen_task("NoSuchScreen") == 1);
    expect("screen_task_not_found_no_activation", unused_screen_activate_calls == 0);

    if (failures != 0) {
        std::printf("command_dispatch_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("command_dispatch_smoke: all checks passed\n");
    return 0;
}
