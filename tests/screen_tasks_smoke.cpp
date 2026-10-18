// Local behavior checks for the ScreenMainMenu screen task
// (0x0050b920 and its task object's vtable at 0x0082f91c; see
// architecture/startup_path.md). Game-space globals and a fake
// nested-allocator object get test-local definitions, the same way
// tests/task_system_smoke.cpp already does for submit_task.
#include "presentation/ScreenTasks.h"
#include "presentation/CommandDispatch.h"
#include "presentation/TaskSystem.h"
#include "platform/HandleRegistry.h"
#include "runtime/FrameClock.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#define HP6_STDCALL __stdcall
#else
#define HP6_STDCALL
#endif

// ---- hp6::platform::service_holder's dependencies (unused by this
// test's own calls, but pulled in transitively through
// HandleRegistry.obj; see tests/handle_registry_smoke.cpp). ----------
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

// ---- ScreenTasks.cpp's own globals ------------------------------------
void* DAT_00fcadf0 = nullptr; // PresentationTaskHandler
void* DAT_00fcae8c = nullptr; // current ScreenMainMenu task
void* DAT_00fcae94 = nullptr; // current ScreenTitlePage task
void* DAT_00fcaed4 = nullptr; // current ScreenFMV task
void* DAT_00fcaeac = nullptr; // current ScreenAttractMode task
void* DAT_00fcae14 = nullptr; // current ScreenLanguageSelect task
std::uint8_t DAT_00cef67a = 0;
std::int32_t DAT_00868814 = 0;
std::uint8_t DAT_00cef53d = 0;
bool check_for_input() { return false; }

// ---- FrameClock.cpp's own globals (pulled in transitively because
// this test calls hp6::runtime::current_tick_value()) -----------------
void* DAT_00cef730 = nullptr;
std::uint8_t DAT_00cef72c = 0;
float DAT_00ba9504 = 0.0f;
float DAT_008482ec = 65536.0f;
void* DAT_0089eb0c = nullptr;
void* DAT_00fc7f90 = nullptr;
std::int32_t DAT_0089eb14 = 0;
std::int32_t DAT_00cef790 = 0;
std::uint32_t DAT_00d64570 = 0;
std::int32_t DAT_00d64574 = 0;
std::uint32_t DAT_00d64578 = 0;
std::int32_t DAT_00d6457c = 0;
std::int32_t DAT_00d644f0 = 0;
std::int32_t DAT_00d644f4 = 0;
float DAT_0080c680 = 1.0f;
float DAT_0083b230 = 1.0f;
float DAT_008003ac = 1.0f;

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

// ---- fake PresentationTaskHandler + its nested allocator --------------
int node_allocate_calls = 0;
std::uint8_t node_arena[16][64];
int next_node = 0;
void* HP6_STDCALL fake_nested_allocate(std::int32_t size, const void* descriptor) {
    (void)size;
    (void)descriptor;
    ++node_allocate_calls;
    void* slot = node_arena[next_node];
    ++next_node;
    return slot;
}
void* nested_allocator_vtable[1] = {reinterpret_cast<void*>(&fake_nested_allocate)};
struct FakeNestedAllocator {
    void* vtable = nested_allocator_vtable;
};
FakeNestedAllocator fake_nested_allocator;

struct FakeTaskHandler {
    std::uint8_t pad[8] = {};
    void* allocator = &fake_nested_allocator;
    void* queue_head = nullptr;
};
FakeTaskHandler fake_task_handler;

std::int32_t unused_command_handler(const char*) { return 0; }

// ---- real command handlers for AutosaveWarningMessageTask::update's
// three commands, so its branches are actually exercised -------------
std::int32_t is_screen_active_result = 0;
int is_screen_active_calls = 0;
std::int32_t is_screen_active_handler(const char*) {
    ++is_screen_active_calls;
    return is_screen_active_result;
}
int activate_no_focus_calls = 0;
std::int32_t activate_no_focus_handler(const char*) {
    ++activate_no_focus_calls;
    return 0;
}
int deactivate_no_focus_calls = 0;
std::int32_t deactivate_no_focus_handler(const char*) {
    ++deactivate_no_focus_calls;
    return 0;
}
int activate_with_focus_calls = 0;
// A copy, not a stored pointer: `args` points inside
// dispatch_presentation_command's own local stack buffer, which is
// gone by the time the caller can inspect it.
char last_activate_with_focus_args[64] = {};
std::int32_t activate_with_focus_handler(const char* args) {
    ++activate_with_focus_calls;
    std::strncpy(last_activate_with_focus_args, args ? args : "",
                 sizeof(last_activate_with_focus_args) - 1);
    return 0;
}

} // namespace

// Repetition macros rather than a hand-counted literal list: an
// earlier version of this file miscounted and silently left 2 of 93
// slots null, which find_and_invoke_command then dereferenced,
// segfaulting (the same pitfall tests/command_dispatch_smoke.cpp
// documents). X5/X10/X25 make the final counts (93 = 9*X10+X3; 26 =
// X25+1) checkable by arithmetic instead of by eye.
#define HP6_X5(x) x, x, x, x, x
#define HP6_X10(x) HP6_X5(x), HP6_X5(x)
#define HP6_X25(x) HP6_X10(x), HP6_X10(x), HP6_X5(x)

// Pulled in transitively through CommandDispatch.obj (update() calls
// dispatch_presentation_command); most slots don't need a real match
// for this test, just every slot non-null — except the four
// AutosaveWarningMessageTask::update actually dispatches.
using hp6::presentation::CommandTableEntry;
const CommandTableEntry kUnusedCommand{"", &unused_command_handler};
const CommandTableEntry kIsScreenActive{"Presentation.Screens.IsScreenActive",
                                        &is_screen_active_handler};
const CommandTableEntry kActivateNoFocus{"Presentation.Screens.ActivateScreenNoFocus",
                                         &activate_no_focus_handler};
const CommandTableEntry kDeactivateNoFocus{"Presentation.Screens.DeactivateScreenNoFocus",
                                           &deactivate_no_focus_handler};
const CommandTableEntry kActivateWithFocus{"Presentation.Screens.ActivateScreenWithFocus",
                                           &activate_with_focus_handler};
extern const CommandTableEntry* const PTR_PTR_00876ab0[93] = {
    &kIsScreenActive, &kActivateNoFocus, &kDeactivateNoFocus, &kActivateWithFocus,
    HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand),
    HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand),
    HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand),
    &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand,
    &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand,
};

using hp6::presentation::ScreenTaskEntry;
void* unused_screen_vtable[1] = {};
const ScreenTaskEntry kUnusedScreen{unused_screen_vtable, ""};
extern const ScreenTaskEntry* const PTR_PTR_00870948[26] = {
    HP6_X25(&kUnusedScreen), &kUnusedScreen,
};

int main() {
    using namespace hp6::presentation;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // ---- update_main_menu_screen_task (the task's own Update) -------------
    // Only `state`, at byte offset 0xc of the 16-byte task object, is
    // touched by update() (vtable/ready_flag/secondary_interface are
    // not read), so a bare zero-initialized 16-byte buffer is enough.
    {
        std::uint8_t task[16] = {};
        auto& state = *reinterpret_cast<std::int32_t*>(task + 0xc);

        state = 0;
        update_main_menu_screen_task(task);
        expect("update_noop_at_state0", state == 0);

        state = 1;
        DAT_00cef67a = 1; // a modal dialog is active
        update_main_menu_screen_task(task);
        expect("update_waits_while_dialog_active", state == 1);

        DAT_00cef67a = 0;
        DAT_00868814 = -1; // unanswered
        update_main_menu_screen_task(task);
        expect("update_waits_while_unanswered", state == 1);

        DAT_00868814 = 0; // user declined
        update_main_menu_screen_task(task);
        expect("update_declined_resets_to_0", state == 0);

        state = 1;
        DAT_00868814 = 1; // user confirmed
        update_main_menu_screen_task(task);
        expect("update_confirmed_advances_to_2", state == 2);

        update_main_menu_screen_task(task);
        expect("update_noop_at_state2", state == 2);
    }

    // ---- enqueue_main_menu_screen_task (0050b920) --------------------------
    DAT_00fcadf0 = &fake_task_handler;
    std::uint8_t self[16] = {};
    enqueue_main_menu_screen_task(self);
    // 2 allocations through the nested allocator: the task object
    // itself (enqueue_main_menu_screen_task) and the queue node that
    // submit_task wraps it in.
    expect("enqueue_allocated_task_and_node", node_allocate_calls == 2);
    expect("enqueue_set_current_task_global", DAT_00fcae8c != nullptr);
    void* self_plus_8 = *reinterpret_cast<void**>(self + 8);
    expect("enqueue_stored_task_at_self_plus_8", self_plus_8 == DAT_00fcae8c);
    expect("enqueue_queued_on_handler", fake_task_handler.queue_head != nullptr);

    // ---- enqueue_title_page_screen_task (0050ba70) -------------------------
    // Its IsReady also constructs and submits a nested
    // Task_IdleInputTimer, so this exercises the nested-allocator path
    // twice over: the ScreenTitlePage task itself, its queue node, the
    // IdleInputTimer task, and ITS queue node — 4 more allocations.
    const int allocations_before_title_page = node_allocate_calls;
    std::uint8_t title_page_self[16] = {};
    enqueue_title_page_screen_task(title_page_self);
    expect("title_page_allocated_task_timer_and_nodes",
           node_allocate_calls - allocations_before_title_page == 4);
    expect("title_page_set_current_task_global", DAT_00fcae94 != nullptr);
    void* title_page_self_plus_8 = *reinterpret_cast<void**>(title_page_self + 8);
    expect("title_page_stored_task_at_self_plus_8",
           title_page_self_plus_8 == DAT_00fcae94);

    // ---- update_autosave_warning_screen_task (0x00429de0) ------------------
    // current_tick_value() stays 0 throughout this test (nothing here
    // calls run_frame_tick), so last_prompt_tick is set to a negative
    // value to simulate a given elapsed time: elapsed = 0 - last_prompt_tick.
    {
        std::uint8_t task[12] = {};
        auto& last_prompt_tick = *reinterpret_cast<std::int32_t*>(task + 8);

        last_prompt_tick = -1000; // elapsed = 1000: below the 2000 threshold
        expect("autosave_waits_before_threshold", update_autosave_warning_screen_task(task) == 0);
        expect("autosave_no_dispatch_before_threshold", is_screen_active_calls == 0);

        last_prompt_tick = -2001; // elapsed = 2001: past the first threshold
        is_screen_active_result = 0; // prompt screen not active yet
        std::int32_t result = update_autosave_warning_screen_task(task);
        expect("autosave_checked_screen_active", is_screen_active_calls == 1);
        expect("autosave_activated_inactive_prompt", activate_no_focus_calls == 1);
        expect("autosave_waits_for_input_or_timeout", result == 0);

        is_screen_active_result = 1; // prompt screen already active
        result = update_autosave_warning_screen_task(task);
        expect("autosave_skips_activate_when_already_active", activate_no_focus_calls == 1);
        expect("autosave_still_waiting_at_8000_threshold", result == 0);

        const int activate_with_focus_calls_before = activate_with_focus_calls;
        last_prompt_tick = -8001; // elapsed = 8001: past the second threshold, still no input
        result = update_autosave_warning_screen_task(task);
        expect("autosave_dispatches_title_page_cut",
               activate_with_focus_calls - activate_with_focus_calls_before == 1);
        expect("autosave_title_page_cut_args",
               std::strcmp(last_activate_with_focus_args, "ScreenTitlePageCut_PC") == 0);
        expect("autosave_returns_done", result == 1);
    }

    // ---- enqueue_autosave_warning_screen_task (0x0050bae0) -----------------
    const int allocations_before_autosave = node_allocate_calls;
    std::uint8_t autosave_self[16] = {};
    enqueue_autosave_warning_screen_task(autosave_self);
    expect("autosave_allocated_task_and_node",
           node_allocate_calls - allocations_before_autosave == 2);
    void* autosave_self_plus_8 = *reinterpret_cast<void**>(autosave_self + 8);
    expect("autosave_stored_task_at_self_plus_8", autosave_self_plus_8 != nullptr);

    // ---- enqueue_fmv_screen_task (0x0050bcd0) ------------------------------
    // Only the constructor/destructor are reconstructed for this task
    // (see ScreenTasks.h); this just checks construction succeeds and
    // is wired up to the task handler like every other screen task.
    const int allocations_before_fmv = node_allocate_calls;
    std::uint8_t fmv_self[16] = {};
    enqueue_fmv_screen_task(fmv_self);
    expect("fmv_allocated_task_and_node", node_allocate_calls - allocations_before_fmv == 2);
    expect("fmv_set_current_task_global", DAT_00fcaed4 != nullptr);
    void* fmv_self_plus_8 = *reinterpret_cast<void**>(fmv_self + 8);
    expect("fmv_stored_task_at_self_plus_8", fmv_self_plus_8 == DAT_00fcaed4);

    // ---- enqueue_attract_mode_screen_task (0x0050bc60) ---------------------
    const int allocations_before_attract = node_allocate_calls;
    std::uint8_t attract_self[16] = {};
    enqueue_attract_mode_screen_task(attract_self);
    expect("attract_allocated_task_and_node",
           node_allocate_calls - allocations_before_attract == 2);
    expect("attract_set_current_task_global", DAT_00fcaeac != nullptr);
    void* attract_self_plus_8 = *reinterpret_cast<void**>(attract_self + 8);
    expect("attract_stored_task_at_self_plus_8", attract_self_plus_8 == DAT_00fcaeac);

    // ---- enqueue_language_select_screen_task (0x0050b8b0) ------------------
    const int allocations_before_lang = node_allocate_calls;
    std::uint8_t lang_self[16] = {};
    enqueue_language_select_screen_task(lang_self);
    expect("lang_allocated_task_and_node", node_allocate_calls - allocations_before_lang == 2);
    expect("lang_set_current_task_global", DAT_00fcae14 != nullptr);
    void* lang_self_plus_8 = *reinterpret_cast<void**>(lang_self + 8);
    expect("lang_stored_task_at_self_plus_8", lang_self_plus_8 == DAT_00fcae14);

    if (failures != 0) {
        std::printf("screen_tasks_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("screen_tasks_smoke: all checks passed\n");
    return 0;
}
