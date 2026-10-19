#include "presentation/ScreenTasks.h"
#include "presentation/CommandDispatch.h"
#include "presentation/TaskSystem.h"
#include "platform/HandleRegistry.h"
#include "runtime/FrameClock.h"

#include <cstdint>

// Addresses read/written by the binary but absent from the exported
// globals; tests provide their own definitions the way the other
// src/presentation/src/platform files already do.
extern void* DAT_00fcadf0;    // PresentationTaskHandler
extern void* DAT_00fcae8c;    // the current ScreenMainMenu task, or null
extern void* DAT_00fcae94;    // the current ScreenTitlePage task, or null
extern void* DAT_00fcaed4;    // the current ScreenFMV task, or null
extern void* DAT_00fcaeac;    // the current ScreenAttractMode task, or null
extern void* DAT_00fcae14;    // the current ScreenLanguageSelect task, or null
extern std::uint8_t DAT_00cef67a; // modal-dialog-active flag (0x005d3070's GUI_CONFIRMSAVE)
extern std::int32_t DAT_00868814; // dialog response: 0 = no, 1 = yes, else unanswered
extern std::uint8_t DAT_00cef53d; // snapshotted by Task_IdleInputTimer's destructor

// 0x004ccca0 — not reconstructed: an external dependency, same status
// as 0x006d95d0 (src/platform/StorageInit.cpp). Reports whether a
// keyboard/controller input was detected since it was last asked.
extern bool check_for_input();

namespace hp6::presentation {
namespace {

// This class's methods below are called through a vtable, exactly
// like the game's own task objects (submit_task genuinely invokes
// vtable slot 1 in __thiscall, since it was reconstructed to match
// the real binary). MSVC does not allow declaring a free function
// __thiscall, so the methods are plain non-static members instead —
// the compiler already generates __thiscall for those — and their
// addresses are taken as raw code pointers via the member-pointer/
// pointer union below, a standard, well-known MSVC implementation
// fact for a non-virtual member of a class with no multiple/virtual
// inheritance (true here: this class has no base classes at all).
struct ScreenMainMenuTask {
    void* vtable;
    std::uint8_t ready_flag;
    std::uint8_t reserved[3];
    void* secondary_interface;
    std::int32_t state;

    // Shared by every "Presentation:TaskHandler::Task"-tagged object
    // (confirmed identical across Task_Bootup/Task_FrontEndChapter/
    // this one in the exported x86): restores the secondary-interface
    // field to its pre-construction value, clears the owning global,
    // stamps the shared "destroyed" vtable sentinel, and conditionally
    // releases the memory through release_through_service.
    void* destroy(bool should_free) {
        secondary_interface = reinterpret_cast<void*>(0x82f940);
        DAT_00fcae8c = nullptr;
        vtable = reinterpret_cast<void*>(0x83d174);
        if (should_free) {
            hp6::platform::release_through_service(this);
        }
        return this;
    }

    // Unconditionally resets the state to 0 (abandon/restart) and
    // reports "ready" — this class never leaves its ready_flag for
    // submit_task to set, since this always returns a nonzero result.
    std::int32_t query_ready() {
        state = 0;
        return 1;
    }

    // update_main_menu_screen_task's real body; see ScreenTasks.h.
    void update() {
        if (state != 1) {
            return;
        }
        if (DAT_00cef67a == 1) {
            return; // a modal dialog is active; wait
        }
        if (DAT_00868814 == 0) {
            state = 0; // user declined; abandon
            return;
        }
        if (DAT_00868814 != 1) {
            return; // not yet answered
        }
        dispatch_presentation_command(
            "Presentation.Screens.ActivateScreenWithFocus ScreenTitlePage");
        state = 2;
    }
};

// Forward declaration: ScreenTitlePageTask::query_ready constructs a
// Task_IdleInputTimer and needs its vtable before the vtable arrays
// are defined further down this file.
extern void* kIdleInputTimerTaskVtable[4];

// "Presentation:TaskHandler::Task_IdleInputTimer" (0x00821508),
// constructed by ScreenTitlePageTask::query_ready below. See
// ScreenTasks.h for the full behavioural description.
struct IdleInputTimerTask {
    void* vtable;
    std::uint8_t ready_flag;
    std::uint8_t reserved[3];
    std::int32_t last_input_tick; // +0x8
    std::int32_t state;           // +0xc: -1 (not yet armed) or 0 = watch for input

    void* destroy(bool should_free) {
        state = DAT_00cef53d;
        vtable = reinterpret_cast<void*>(0x83d174);
        if (should_free) {
            hp6::platform::release_through_service(this);
        }
        return this;
    }

    std::int32_t query_ready() {
        last_input_tick = hp6::runtime::current_tick_value();
        return 1;
    }

    void update() {
        if (state != -1 && state != 0) {
            return;
        }
        if (check_for_input()) {
            last_input_tick = hp6::runtime::current_tick_value();
        }
    }
};

// 24-byte "Presentation:TaskHandler::Task" for ScreenTitlePage; see
// ScreenTasks.h. Slots 2 ("is self enqueued in some list, tracked via
// +0x14") and 3 ("Update") are not reconstructed.
struct ScreenTitlePageTask {
    void* vtable;
    std::uint8_t ready_flag;
    std::uint8_t reserved[3];
    void* secondary_interface;
    std::uint8_t unresolved_pair[2]; // +0xc/+0xd, cleared by IsReady
    std::uint8_t unresolved_pad[2];
    std::int32_t unused_by_ready;    // +0x10, cleared by IsReady
    void* idle_timer_task;           // +0x14

    void* destroy(bool should_free) {
        secondary_interface = reinterpret_cast<void*>(0x82f968);
        DAT_00fcae94 = nullptr;
        vtable = reinterpret_cast<void*>(0x83d174);
        if (should_free) {
            hp6::platform::release_through_service(this);
        }
        return this;
    }

    // Constructs and submits the Task_IdleInputTimer sub-task (see
    // IdleInputTimerTask above), then always reports "ready".
    std::int32_t query_ready() {
        unresolved_pair[0] = 0;
        unresolved_pair[1] = 0;
        auto* timer = static_cast<IdleInputTimerTask*>(allocate_via_task_handler(
            DAT_00fcadf0, sizeof(IdleInputTimerTask),
            "Presentation:TaskHandler::Task_IdleInputTimer"));
        if (timer != nullptr) {
            timer->ready_flag = 0;
            timer->vtable = kIdleInputTimerTaskVtable;
        }
        idle_timer_task = submit_task(DAT_00fcadf0, timer);
        unused_by_ready = 0;
        return 1;
    }
};

template <typename Member>
void* member_function_address(Member member) {
    union {
        Member as_member;
        void* as_raw;
    } converter{};
    converter.as_member = member;
    return converter.as_raw;
}

// A shared no-op stub (0x00405a30: a bare RET) reused across unrelated
// task vtables for a slot this class does not use; takes no
// parameters at all, so no calling-convention adapter is needed.
void unused_screen_task_slot() {}

void* kMainMenuScreenTaskVtable[4] = {
    member_function_address(&ScreenMainMenuTask::destroy),
    member_function_address(&ScreenMainMenuTask::query_ready),
    reinterpret_cast<void*>(&unused_screen_task_slot),
    member_function_address(&ScreenMainMenuTask::update),
};

void* kIdleInputTimerTaskVtable[4] = {
    member_function_address(&IdleInputTimerTask::destroy),
    member_function_address(&IdleInputTimerTask::query_ready),
    reinterpret_cast<void*>(&unused_screen_task_slot),
    member_function_address(&IdleInputTimerTask::update),
};

// 12-byte "Presentation:TaskHandler::Task" for
// ScreenAutosaveWarningMessage; see ScreenTasks.h. Simpler than
// ScreenMainMenu/ScreenTitlePage: no secondary interface, no dedicated
// "current task" global.
struct AutosaveWarningMessageTask {
    void* vtable;
    std::uint8_t ready_flag;
    std::uint8_t reserved[3];
    std::int32_t last_prompt_tick; // +0x8: tick/3 snapshot at construction

    void* destroy(bool should_free) {
        vtable = reinterpret_cast<void*>(0x83d174);
        if (should_free) {
            hp6::platform::release_through_service(this);
        }
        return this;
    }

    std::int32_t query_ready() {
        last_prompt_tick = hp6::runtime::current_tick_value() / 3;
        return 1;
    }

    // Vtable slot 2; not called from this project, reconstructed for
    // completeness since it is small and clear.
    void deactivate_prompt_screen() {
        dispatch_presentation_command(
            "Presentation.Screens.DeactivateScreenNoFocus ScreenAutosaveWarningMessagePrompt");
    }

    std::int32_t update() {
        const bool input_detected = check_for_input();
        const std::int32_t elapsed = hp6::runtime::current_tick_value() / 3 - last_prompt_tick;
        if (elapsed <= 0x7d0) {
            return 0;
        }
        if (dispatch_presentation_command(
                "Presentation.Screens.IsScreenActive ScreenAutosaveWarningMessagePrompt") == 0) {
            dispatch_presentation_command(
                "Presentation.Screens.ActivateScreenNoFocus ScreenAutosaveWarningMessagePrompt");
        }
        if (input_detected) {
            return 1;
        }
        if (elapsed <= 0x1f40) {
            return 0;
        }
        dispatch_presentation_command(
            "Presentation.Screens.ActivateScreenWithFocus ScreenTitlePageCut_PC");
        return 1;
    }
};

// 44-byte "Presentation:TaskHandler::Task" for ScreenFMV; see
// ScreenTasks.h. Only the constructor/destructor are reconstructed —
// everything past field+8 (vtable slots 1-3, substantially more
// complex than every other screen task here) is left as raw,
// unresolved storage.
struct ScreenFMVTask {
    void* vtable;
    std::uint8_t ready_flag;
    std::uint8_t reserved[3];
    void* secondary_interface;
    std::uint8_t unresolved[0x2c - 0xc];

    void* destroy(bool should_free) {
        secondary_interface = reinterpret_cast<void*>(0x82fa00);
        DAT_00fcaed4 = nullptr;
        vtable = reinterpret_cast<void*>(0x83d174);
        if (should_free) {
            hp6::platform::release_through_service(this);
        }
        return this;
    }
};

// 48-byte "Presentation:TaskHandler::Task" for ScreenAttractMode; see
// ScreenTasks.h. Only the constructor/destructor are reconstructed.
struct ScreenAttractModeTask {
    void* vtable;
    std::uint8_t ready_flag;
    std::uint8_t reserved[3];
    void* secondary_interface;
    std::uint8_t unresolved[0x30 - 0xc];

    void* destroy(bool should_free) {
        secondary_interface = reinterpret_cast<void*>(0x82fa28);
        DAT_00fcaeac = nullptr;
        vtable = reinterpret_cast<void*>(0x83d174);
        if (should_free) {
            hp6::platform::release_through_service(this);
        }
        return this;
    }
};

// 24-byte "Presentation:TaskHandler::Task" for ScreenLanguageSelect;
// see ScreenTasks.h. The constructor, destructor, IsReady (slot 1),
// and slot 2 are reconstructed; slot 3 ("Update") is not.
struct ScreenLanguageSelectTask {
    void* vtable;
    std::uint8_t ready_flag;
    std::uint8_t reserved[3];
    void* secondary_interface;
    void* pending_task;        // +0xc: Update's own Task_IdleInputTimer submission (not reconstructed)
    std::uint8_t unresolved[4]; // +0x10..+0x13: touched only by Update, not reconstructed
    std::int32_t state;         // +0x14: Update's own state-machine phase

    void* destroy(bool should_free) {
        // The exported x86 writes 0x821548 then immediately overwrites
        // it with 0x82154c; only the final value is observable.
        secondary_interface = reinterpret_cast<void*>(0x82154c);
        DAT_00fcae14 = nullptr;
        vtable = reinterpret_cast<void*>(0x83d174);
        if (should_free) {
            hp6::platform::release_through_service(this);
        }
        return this;
    }

    // Vtable slot 1 ("IsReady"); see ScreenTasks.h.
    std::int32_t query_ready() {
        state = 0;
        return 1;
    }

    // Vtable slot 2; see ScreenTasks.h.
    void mark_pending_task_ready_if_enqueued() {
        auto* node = *reinterpret_cast<std::uint8_t**>(
            static_cast<std::uint8_t*>(DAT_00fcadf0) + 0xc);
        while (node != nullptr) {
            if (*reinterpret_cast<void**>(node + 8) == pending_task) {
                static_cast<std::uint8_t*>(pending_task)[4] = 1;
                return;
            }
            node = *reinterpret_cast<std::uint8_t**>(node + 4);
        }
    }
};

// Slots 2/3 are not reconstructed (see ScreenTitlePageTask above); the
// shared no-op stub keeps the vtable fully populated and safe to call
// through, even though nothing in this project currently does.
void* kTitlePageScreenTaskVtable[4] = {
    member_function_address(&ScreenTitlePageTask::destroy),
    member_function_address(&ScreenTitlePageTask::query_ready),
    reinterpret_cast<void*>(&unused_screen_task_slot),
    reinterpret_cast<void*>(&unused_screen_task_slot),
};

void* kAutosaveWarningScreenTaskVtable[4] = {
    member_function_address(&AutosaveWarningMessageTask::destroy),
    member_function_address(&AutosaveWarningMessageTask::query_ready),
    member_function_address(&AutosaveWarningMessageTask::deactivate_prompt_screen),
    member_function_address(&AutosaveWarningMessageTask::update),
};

// Slots 1-3 are not reconstructed (see ScreenFMVTask above).
void* kFMVScreenTaskVtable[4] = {
    member_function_address(&ScreenFMVTask::destroy),
    reinterpret_cast<void*>(&unused_screen_task_slot),
    reinterpret_cast<void*>(&unused_screen_task_slot),
    reinterpret_cast<void*>(&unused_screen_task_slot),
};

// Slots 1-3 are not reconstructed (see ScreenAttractModeTask above).
void* kAttractModeScreenTaskVtable[4] = {
    member_function_address(&ScreenAttractModeTask::destroy),
    reinterpret_cast<void*>(&unused_screen_task_slot),
    reinterpret_cast<void*>(&unused_screen_task_slot),
    reinterpret_cast<void*>(&unused_screen_task_slot),
};

// Slot 3 ("Update") is not reconstructed (see ScreenLanguageSelectTask
// above).
void* kLanguageSelectScreenTaskVtable[4] = {
    member_function_address(&ScreenLanguageSelectTask::destroy),
    member_function_address(&ScreenLanguageSelectTask::query_ready),
    member_function_address(&ScreenLanguageSelectTask::mark_pending_task_ready_if_enqueued),
    reinterpret_cast<void*>(&unused_screen_task_slot),
};

} // namespace

void enqueue_main_menu_screen_task(void* self) {
    void* task_handler = DAT_00fcadf0;
    auto* task = static_cast<ScreenMainMenuTask*>(allocate_via_task_handler(
        task_handler, sizeof(ScreenMainMenuTask), "Presentation:TaskHandler::Task"));

    void* submitted = nullptr;
    if (task != nullptr) {
        task->ready_flag = 0;
        task->secondary_interface = reinterpret_cast<void*>(0x82f940);
        DAT_00fcae8c = task;
        task->vtable = kMainMenuScreenTaskVtable;
        task->secondary_interface = reinterpret_cast<void*>(0x82f93c);
        submitted = submit_task(task_handler, task);
    } else {
        submitted = submit_task(task_handler, nullptr);
    }
    *reinterpret_cast<void**>(static_cast<std::uint8_t*>(self) + 8) = submitted;
}

void update_main_menu_screen_task(void* task_ptr) {
    static_cast<ScreenMainMenuTask*>(task_ptr)->update();
}

void enqueue_title_page_screen_task(void* self) {
    void* task_handler = DAT_00fcadf0;
    auto* task = static_cast<ScreenTitlePageTask*>(allocate_via_task_handler(
        task_handler, sizeof(ScreenTitlePageTask), "Presentation:TaskHandler::Task"));

    void* submitted = nullptr;
    if (task != nullptr) {
        task->ready_flag = 0;
        task->secondary_interface = reinterpret_cast<void*>(0x82f968);
        DAT_00fcae94 = task;
        task->vtable = kTitlePageScreenTaskVtable;
        task->secondary_interface = reinterpret_cast<void*>(0x82f964);
        submitted = submit_task(task_handler, task);
    } else {
        submitted = submit_task(task_handler, nullptr);
    }
    *reinterpret_cast<void**>(static_cast<std::uint8_t*>(self) + 8) = submitted;
}

void enqueue_autosave_warning_screen_task(void* self) {
    void* task_handler = DAT_00fcadf0;
    auto* task = static_cast<AutosaveWarningMessageTask*>(allocate_via_task_handler(
        task_handler, sizeof(AutosaveWarningMessageTask), "Presentation:TaskHandler::Task"));

    void* submitted = nullptr;
    if (task != nullptr) {
        task->ready_flag = 0;
        task->vtable = kAutosaveWarningScreenTaskVtable;
        submitted = submit_task(task_handler, task);
    } else {
        submitted = submit_task(task_handler, nullptr);
    }
    *reinterpret_cast<void**>(static_cast<std::uint8_t*>(self) + 8) = submitted;
}

std::int32_t update_autosave_warning_screen_task(void* task_ptr) {
    return static_cast<AutosaveWarningMessageTask*>(task_ptr)->update();
}

void enqueue_fmv_screen_task(void* self) {
    void* task_handler = DAT_00fcadf0;
    auto* task = static_cast<ScreenFMVTask*>(allocate_via_task_handler(
        task_handler, sizeof(ScreenFMVTask), "Presentation:TaskHandler::Task"));

    void* submitted = nullptr;
    if (task != nullptr) {
        task->ready_flag = 0;
        task->secondary_interface = reinterpret_cast<void*>(0x82fa00);
        DAT_00fcaed4 = task;
        task->vtable = kFMVScreenTaskVtable;
        task->secondary_interface = reinterpret_cast<void*>(0x82f9fc);
        submitted = submit_task(task_handler, task);
    } else {
        submitted = submit_task(task_handler, nullptr);
    }
    *reinterpret_cast<void**>(static_cast<std::uint8_t*>(self) + 8) = submitted;
}

void enqueue_attract_mode_screen_task(void* self) {
    void* task_handler = DAT_00fcadf0;
    auto* task = static_cast<ScreenAttractModeTask*>(allocate_via_task_handler(
        task_handler, sizeof(ScreenAttractModeTask), "Presentation:TaskHandler::Task"));

    void* submitted = nullptr;
    if (task != nullptr) {
        task->ready_flag = 0;
        task->secondary_interface = reinterpret_cast<void*>(0x82fa28);
        DAT_00fcaeac = task;
        task->vtable = kAttractModeScreenTaskVtable;
        task->secondary_interface = reinterpret_cast<void*>(0x82fa24);
        submitted = submit_task(task_handler, task);
    } else {
        submitted = submit_task(task_handler, nullptr);
    }
    *reinterpret_cast<void**>(static_cast<std::uint8_t*>(self) + 8) = submitted;
}

void enqueue_language_select_screen_task(void* self) {
    void* task_handler = DAT_00fcadf0;
    auto* task = static_cast<ScreenLanguageSelectTask*>(allocate_via_task_handler(
        task_handler, sizeof(ScreenLanguageSelectTask), "Presentation:TaskHandler::Task"));

    void* submitted = nullptr;
    if (task != nullptr) {
        task->ready_flag = 0;
        task->secondary_interface = reinterpret_cast<void*>(0x82154c);
        DAT_00fcae14 = task;
        task->vtable = kLanguageSelectScreenTaskVtable;
        task->secondary_interface = reinterpret_cast<void*>(0x821548);
        submitted = submit_task(task_handler, task);
    } else {
        submitted = submit_task(task_handler, nullptr);
    }
    *reinterpret_cast<void**>(static_cast<std::uint8_t*>(self) + 8) = submitted;
}

std::int32_t query_ready_language_select_screen_task(void* task_ptr) {
    return static_cast<ScreenLanguageSelectTask*>(task_ptr)->query_ready();
}

void mark_language_select_pending_task_ready(void* task_ptr) {
    static_cast<ScreenLanguageSelectTask*>(task_ptr)->mark_pending_task_ready_if_enqueued();
}

} // namespace hp6::presentation
