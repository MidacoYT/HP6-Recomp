// Local behavior checks for the ScreenMainMenu screen task
// (0x0050b920 and its task object's vtable at 0x0082f91c; see
// architecture/startup_path.md). Game-space globals and a fake
// nested-allocator object get test-local definitions, the same way
// tests/task_system_smoke.cpp already does for submit_task.
#include "presentation/ScreenTasks.h"
#include "presentation/ChapterFlow.h"
#include "presentation/CommandDispatch.h"
#include "presentation/GuiTextTable.h"
#include "presentation/TaskSystem.h"
#include "platform/HandleRegistry.h"
#include "runtime/FrameClock.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>

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

// ---- ScreenLanguageSelectTask::update's own globals (0x00429890) ---------
// The language-menu state machine's dependencies. DAT_00fcae04's service
// object is laid out per scenario in main() (its +0x04 name buffer,
// +0x98 platform marker, and the +0xb8/+0xbc narrow language-name vector
// are what update and the confirm path read).
void* DAT_00cef610 = nullptr; // chapter-load startup context
std::uint8_t lang_service_object[0xc0] = {};
void* DAT_00fcae04 = nullptr;
std::int32_t DAT_00879654 = -1;
std::uint32_t DAT_00d6b8a0 = 0;
std::int32_t DAT_00d6b898 = 0;
std::int32_t DAT_00d6e5b8 = 0;
void* DAT_00d46aa8 = nullptr;
std::int32_t DAT_00d472dc = -1;

// The wide " " constant at 0x0081fca8, mirrored like the other
// game-space constants. `extern` marks the external linkage the src
// side's extern declaration already promises (without it, a const
// namespace-scope variable defaults to internal linkage in C++).
extern const wchar_t kWideBlank_0081fca8[2];
const wchar_t kWideBlank_0081fca8[2] = {L' ', L'\0'};

// ---- ChapterFlow.cpp's globals (pulled in because the language-select
// confirm path calls resolve_and_load_chapter) — the same definitions
// tests/chapter_flow_smoke.cpp provides -----------------------------------
std::uint32_t DAT_00fd1b14 = 0;
void* DAT_00fcd530 = nullptr;
std::uint8_t* DAT_00fcd534 = nullptr;
std::uint8_t DAT_0087e014 = 1;
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

// ---- the deferred 0x005e5e40 language-text lookup ------------------------
// The real FUN_005e5e40 finds the wide display text mapped for a narrow
// language name inside the language service's text map (inserting an
// empty value when the key is absent and returning its storage). This
// stand-in records its calls and owns the wide strings the game's own
// map would hold.
int lookup_language_text_calls = 0;
void* last_lookup_owner = nullptr;
const wchar_t lookup_en[8] = L"English";  // 7 chars + NUL
const wchar_t lookup_fr[7] = L"French";   // 6 chars + NUL
const wchar_t lookup_empty[1] = L"";
// (ECX/name argument, void* owner): the real call passes the service's
// own +0x04 name buffer as the map owner.
const wchar_t* lookup_localized_language_text(const char* language_name,
                                              void* owner) {
    ++lookup_language_text_calls;
    last_lookup_owner = owner;
    if (std::strcmp(language_name, "English") == 0) {
        return lookup_en;
    }
    if (std::strcmp(language_name, "French") == 0) {
        return lookup_fr;
    }
    return lookup_empty;
}

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

// ---- fake PresentationTaskHandler + its nested allocator --------------
// 24 slots: the original 16 covered the pre-LanguageSelect tests
// exactly; update_language_select_screen_task's two state-1 runs
// allocate 4 more (each run submits its own IdleInputTimer + queue
// node).
int node_allocate_calls = 0;
std::uint8_t node_arena[24][64];
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

    // ---- query_ready_language_select_screen_task (0x00429850) --------------
    {
        std::uint8_t task[0x18] = {};
        auto& state = *reinterpret_cast<std::int32_t*>(task + 0x14);
        state = 2; // mid-navigation
        expect("lang_ready_returns_true", query_ready_language_select_screen_task(task) == 1);
        expect("lang_ready_resets_state", state == 0);
    }

    // ---- mark_language_select_pending_task_ready (0x00429860) --------------
    // PresentationTaskHandler's own task-list head lives at +0xc of the
    // same fake_task_handler object used above (FakeTaskHandler::queue_head).
    {
        std::uint8_t task[0x18] = {};
        auto& pending_task = *reinterpret_cast<void**>(task + 0xc);
        std::uint8_t pending[8] = {}; // +4 = ready_flag, matching every task's own layout

        // No matching node enqueued: nothing happens.
        pending_task = pending;
        fake_task_handler.queue_head = nullptr;
        mark_language_select_pending_task_ready(task);
        expect("lang_slot2_noop_when_not_enqueued", pending[4] == 0);

        // A node tagged with a different pointer: still no match.
        std::uint8_t other_node[12] = {};
        std::uint8_t unrelated[4] = {};
        *reinterpret_cast<void**>(other_node + 4) = nullptr; // next
        *reinterpret_cast<void**>(other_node + 8) = unrelated; // tag
        fake_task_handler.queue_head = other_node;
        mark_language_select_pending_task_ready(task);
        expect("lang_slot2_noop_when_tag_mismatches", pending[4] == 0);

        // A second node (reached via the first node's `next`) tagged with
        // the real pending task pointer: its ready_flag gets set.
        std::uint8_t matching_node[12] = {};
        *reinterpret_cast<void**>(matching_node + 4) = nullptr; // next
        *reinterpret_cast<void**>(matching_node + 8) = pending; // tag
        *reinterpret_cast<void**>(other_node + 4) = matching_node; // chain
        mark_language_select_pending_task_ready(task);
        expect("lang_slot2_marks_ready_when_enqueued", pending[4] == 1);
    }

    // ---- update_language_select_screen_task (0x00429890) --------------------
    // current_tick_value() stays 0 throughout (nothing here calls
    // run_frame_tick), so elapsing time is simulated the same way the
    // autosave test does: by writing a negative tick snapshot into the
    // task's +0x10 field (elapsed = 0/3 - snapshot/3; -1002 gives
    // exactly 334, first value past the 0x14d = 333 threshold).
    DAT_00fcadf0 = &fake_task_handler;
    fake_task_handler.queue_head = nullptr;
    DAT_00fcae04 = lang_service_object;
    std::memset(lang_service_object, 0, sizeof(lang_service_object));

    // State 0: the chapter-load gate.
    {
        std::uint8_t task[0x18] = {};
        auto& state = *reinterpret_cast<std::int32_t*>(task + 0x14);
        state = 0;
        DAT_00cef610 = nullptr;
        expect("lang_update_state0_waits_without_startup_context",
               update_language_select_screen_task(task) == 0);
        expect("lang_update_state0_stays_without_startup_context", state == 0);

        std::uint8_t startup_context[0xb1] = {};
        startup_context[0xb0] = 1; // the ChapterLoad state machine is mid-load
        DAT_00cef610 = startup_context;
        update_language_select_screen_task(task);
        expect("lang_update_state0_stays_while_load_pending", state == 0);

        startup_context[0xb0] = 0;
        // The exported x86's state-0 exit carries DAT_00cef610's own
        // value in EAX (see ScreenTasks.h): masked to its own low bits.
        expect("lang_update_state0_advances_when_gate_clears",
               update_language_select_screen_task(task) ==
                   static_cast<std::int32_t>(
                       reinterpret_cast<std::uintptr_t>(startup_context) &
                       0xffffff00u));
        expect("lang_update_state0_advanced_to_state_1", state == 1);
    }
    DAT_00cef610 = nullptr;

    // State 1: arm the idle timer, then fill the 16 menu keys.
    {
        hp6::presentation::clear_localized_text_tables();
        // Two installed languages, narrow entries.
        const char* lang_names[2] = {"English", "French"};
        *reinterpret_cast<const void**>(lang_service_object + 0xb8) = lang_names;
        *reinterpret_cast<const void**>(lang_service_object + 0xbc) =
            lang_names + 2;

        const int allocations_before = node_allocate_calls;
        std::uint8_t task[0x18] = {};
        auto& state = *reinterpret_cast<std::int32_t*>(task + 0x14);
        auto& snapshot = *reinterpret_cast<std::int32_t*>(task + 0x10);
        state = 1;
        update_language_select_screen_task(task);
        expect("lang_update_state1_submitted_idle_timer",
               node_allocate_calls - allocations_before == 2);
        expect("lang_update_state1_snapshots_tick", snapshot == 0);
        expect("lang_update_state1_stores_pending_task",
               *reinterpret_cast<void**>(task + 0xc) != nullptr);
        // The submitted IdleInputTimer rides the handler's own queue
        // node (+8 tag = the task): its ready flag (+4) is 0 and its
        // own state (+0xc) is the -1 "not yet armed" sentinel.
        auto* timer =
            fake_task_handler.queue_head != nullptr
                ? *reinterpret_cast<std::uint8_t**>(
                      static_cast<std::uint8_t*>(fake_task_handler.queue_head) + 8)
                : nullptr;
        expect("lang_update_state1_timer_in_handler_queue", timer != nullptr);
        expect("lang_update_state1_subtask_ready_flag_clear", timer[4] == 0);
        expect("lang_update_state1_subtask_state_sentinel",
               *reinterpret_cast<std::int32_t*>(timer + 0xc) == -1);
        // 16 keys: in-range slots carry the language display text, the
        // rest the wide-blank fallback.
        for (int i = 0; i < 16; ++i) {
            char key[32];
            std::sprintf(key, "GUI_LANGUAGEMENU%d", i);
            const wchar_t* stored =
                hp6::presentation::read_localized_text(0x00cf8168, key);
            const wchar_t* expected =
                i < 2 ? (i == 0 ? lookup_en : lookup_fr) : kWideBlank_0081fca8;
            expect("lang_update_state1_wrote_gui_languagemenu_key",
                   stored != nullptr && std::wcscmp(stored, expected) == 0);
        }
        expect("lang_update_state1_looked_up_both_languages",
               lookup_language_text_calls == 2);
        expect("lang_update_state1_lookup_keyed_against_name_buffer",
               last_lookup_owner == lang_service_object + 4);
        expect("lang_update_state1_advanced_to_state_2", state == 2);
    }

    // State 1 with an empty language vector: no lookups, all 16 keys
    // get the wide-blank fallback.
    {
        hp6::presentation::clear_localized_text_tables();
        const int lookups_before = lookup_language_text_calls;
        *reinterpret_cast<const char**>(lang_service_object + 0xb8) = nullptr;
        *reinterpret_cast<const char**>(lang_service_object + 0xbc) = nullptr;
        std::uint8_t task[0x18] = {};
        *reinterpret_cast<std::int32_t*>(task + 0x14) = 1;
        update_language_select_screen_task(task);
        expect("lang_update_state1_empty_vector_no_lookups",
               lookup_language_text_calls == lookups_before);
        bool all_blank = true;
        for (int i = 0; i < 16; ++i) {
            char key[32];
            std::sprintf(key, "GUI_LANGUAGEMENU%d", i);
            const wchar_t* stored =
                hp6::presentation::read_localized_text(0x00cf8168, key);
            all_blank = all_blank && stored != nullptr &&
                        std::wcscmp(stored, kWideBlank_0081fca8) == 0;
        }
        expect("lang_update_state1_empty_vector_all_wide_blank", all_blank);
    }
    std::memset(lang_service_object, 0, sizeof(lang_service_object));

    // State 2: flag bookkeeping happens on every state-2 entry, then
    // the first past-threshold update commits the selection through
    // the confirm path (DAT_00d472dc = 8 + resolve_and_load_chapter).
    // This scenario commits with selection index 1 = "French", so the
    // registry's +0xa0 flag bit gets raised (and the name copied into
    // the service's own +0x04 buffer).
    {
        std::uint8_t registry[0xb4] = {};
        DAT_00d46aa8 = registry;
        DAT_00d472dc = -1;
        DAT_00d6b8a0 = 0;
        DAT_00d6b898 = 7;
        DAT_00d6e5b8 = 7;
        DAT_00879654 = 1;
        const char* lang_names[2] = {"English", "French"};
        *reinterpret_cast<const void**>(lang_service_object + 0xb8) = lang_names;
        *reinterpret_cast<const void**>(lang_service_object + 0xbc) =
            lang_names + 2;

        std::uint8_t task[0x18] = {};
        *reinterpret_cast<std::int32_t*>(task + 0x14) = 2;
        *reinterpret_cast<std::int32_t*>(task + 0x10) = -1002; // elapsed = 334
        expect("lang_update_state2_commits_selection",
               update_language_select_screen_task(task) == 1);
        expect("lang_update_state2_resolved_front_end_chapter",
               DAT_00d472dc == 8);
        expect("lang_update_state2_raised_menu_shown_bit", DAT_00d6b8a0 == 1);
        expect("lang_update_state2_zeroed_00d6b898", DAT_00d6b898 == 0);
        expect("lang_update_state2_zeroed_00d6e5b8", DAT_00d6e5b8 == 0);
        expect("lang_update_state2_copied_selected_name",
               std::strcmp(
                   reinterpret_cast<const char*>(lang_service_object + 4),
                   "French") == 0);
        expect("lang_update_state2_non_english_raises_registry_bit",
               *reinterpret_cast<std::uint32_t*>(registry + 0xa0) == 4);
    }

    // State 2 below the commit threshold: identity bookkeeping runs
    // but no resolve_and_load_chapter, and the task stays at state 2.
    {
        DAT_00d472dc = -1;
        DAT_00d6b8a0 = 0;
        DAT_00d6b898 = 3;
        DAT_00d6e5b8 = 3;
        DAT_00879654 = 1;
        const char* lang_names[2] = {"English", "French"};
        *reinterpret_cast<const void**>(lang_service_object + 0xb8) = lang_names;
        *reinterpret_cast<const void**>(lang_service_object + 0xbc) =
            lang_names + 2;

        std::uint8_t task[0x18] = {};
        auto& state = *reinterpret_cast<std::int32_t*>(task + 0x14);
        *reinterpret_cast<std::int32_t*>(task + 0x10) = -1001; // elapsed = 333
        state = 2;
        // The exported x86's not-yet-committed exit carries the elapsed
        // value in EAX: 333 = 0x14d, masked to 0x100 (see ScreenTasks.h).
        expect("lang_update_state2_waits_below_threshold",
               update_language_select_screen_task(task) == 0x100);
        expect("lang_update_state2_wait_keeps_state_2", state == 2);
        expect("lang_update_state2_wait_does_not_commit", DAT_00d472dc == -1);
        expect("lang_update_state2_wait_still_flags_menu_shown",
               DAT_00d6b8a0 == 1);
        expect("lang_update_state2_wait_still_zeroes_00d6b898",
               DAT_00d6b898 == 0);
        expect("lang_update_state2_wait_still_zeroes_00d6e5b8",
               DAT_00d6e5b8 == 0);
    }

    // State 2 with a fresh tick (elapsed = 0): still waiting, nothing
    // beyond the zero-valued bookkeeping.
    {
        DAT_00d472dc = -1;
        std::uint8_t task[0x18] = {};
        *reinterpret_cast<std::int32_t*>(task + 0x14) = 2;
        *reinterpret_cast<std::int32_t*>(task + 0x10) = 0;
        expect("lang_update_state2_fresh_tick_waits",
               update_language_select_screen_task(task) == 0);
        expect("lang_update_state2_fresh_tick_does_not_commit",
               DAT_00d472dc == -1);
    }

    // State 2 committing a non-English name installed as the ONLY
    // language: the single-language branch pins the selection to 0
    // before the commit (the test deliberately starts it misplaced).
    {
        std::uint8_t registry[0xb4] = {};
        DAT_00d472dc = -1;
        DAT_00879654 = 3; // out of range: only the single-language pin fixes it
        *reinterpret_cast<std::uint32_t*>(registry + 0xa0) = 0;
        DAT_00d46aa8 = registry;
        const char* single[1] = {"English"};
        *reinterpret_cast<const void**>(lang_service_object + 0xb8) = single;
        *reinterpret_cast<const void**>(lang_service_object + 0xbc) =
            single + 1;

        std::uint8_t task[0x18] = {};
        *reinterpret_cast<std::int32_t*>(task + 0x14) = 2;
        *reinterpret_cast<std::int32_t*>(task + 0x10) = -1002;
        expect("lang_update_state2_single_language_commits",
               update_language_select_screen_task(task) == 1);
        expect("lang_update_state2_single_language_pins_selection",
               DAT_00879654 == 0);
        expect("lang_update_state2_single_language_copies_pinned_name",
               std::strcmp(
                   reinterpret_cast<const char*>(lang_service_object + 4),
                   "English") == 0);
        expect("lang_update_state2_non_english_marker_still_raises_bit",
               *reinterpret_cast<std::uint32_t*>(registry + 0xa0) == 4);
    }

    // State 2 committing one of the two built-in English markers the
    // exported x86 compares against ("en-GB" at 0x0083e694, "en-US" at
    // 0x0083e68c): the registry's flag bit must stay untouched.
    {
        std::uint8_t registry[0xb4] = {};
        DAT_00d472dc = -1;
        DAT_00879654 = 0;
        *reinterpret_cast<std::uint32_t*>(registry + 0xa0) = 0;
        DAT_00d46aa8 = registry;
        const char* single[1] = {"en-GB"};
        *reinterpret_cast<const void**>(lang_service_object + 0xb8) = single;
        *reinterpret_cast<const void**>(lang_service_object + 0xbc) =
            single + 1;

        std::uint8_t task[0x18] = {};
        *reinterpret_cast<std::int32_t*>(task + 0x14) = 2;
        *reinterpret_cast<std::int32_t*>(task + 0x10) = -1002;
        expect("lang_update_state2_en_gb_commits",
               update_language_select_screen_task(task) == 1);
        expect("lang_update_state2_en_gb_copies_name",
               std::strcmp(
                   reinterpret_cast<const char*>(lang_service_object + 4),
                   "en-GB") == 0);
        expect("lang_update_state2_en_gb_leaves_registry_flag_alone",
               *reinterpret_cast<std::uint32_t*>(registry + 0xa0) == 0);
    }

    // State 2 with the platform marker containing "WBSUB" (the
    // exported x86's other selection-pinning condition, +0x98 buffer).
    {
        std::uint8_t registry[0xb4] = {};
        DAT_00d472dc = -1;
        DAT_00879654 = 9; // out of range: only the WBSUB pin fixes it
        *reinterpret_cast<std::uint32_t*>(registry + 0xa0) = 0;
        DAT_00d46aa8 = registry;
        const char* two[2] = {"English", "French"};
        *reinterpret_cast<const void**>(lang_service_object + 0xb8) = two;
        *reinterpret_cast<const void**>(lang_service_object + 0xbc) = two + 2;
        std::memcpy(lang_service_object + 0x98, "PC-WBSUB", 8);

        std::uint8_t task[0x18] = {};
        *reinterpret_cast<std::int32_t*>(task + 0x14) = 2;
        *reinterpret_cast<std::int32_t*>(task + 0x10) = -1002;
        expect("lang_update_state2_wbsub_commits",
               update_language_select_screen_task(task) == 1);
        expect("lang_update_state2_wbsub_pins_selection_to_0",
               DAT_00879654 == 0);
        expect("lang_update_state2_wbsub_copies_pinned_name",
               std::strcmp(
                   reinterpret_cast<const char*>(lang_service_object + 4),
                   "English") == 0);
    }

    // State 2 committing with no selection at all (-1): the name copy
    // is skipped entirely but the commit itself still runs.
    {
        DAT_00d472dc = -1;
        DAT_00d6b8a0 = 0;
        DAT_00d6b898 = 5;
        DAT_00d6e5b8 = 5;
        DAT_00879654 = -1;
        std::memset(lang_service_object, 0, sizeof(lang_service_object));

        std::uint8_t task[0x18] = {};
        auto& state = *reinterpret_cast<std::int32_t*>(task + 0x14);
        state = 2;
        *reinterpret_cast<std::int32_t*>(task + 0x10) = -1002;
        expect("lang_update_state2_no_selection_still_commits",
               update_language_select_screen_task(task) == 1);
        expect("lang_update_state2_no_selection_skips_name_copy",
               lang_service_object[4] == 0);
        expect("lang_update_state2_no_selection_still_books_flag",
               DAT_00d6b8a0 == 1);
    }

    if (failures != 0) {
        std::printf("screen_tasks_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("screen_tasks_smoke: all checks passed\n");
    return 0;
}
