#include "presentation/ScreenTasks.h"
#include "presentation/CommandDispatch.h"
#include "presentation/ChapterFlow.h"
#include "presentation/GuiTextTable.h"
#include "presentation/TaskSystem.h"
#include "platform/HandleRegistry.h"
#include "runtime/FrameClock.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

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

// ---- ScreenLanguageSelectTask::update's dependencies (0x00429890) --------
extern void* DAT_00cef610;  // chapter-load startup context; its +0xb0 byte gates
                            // state 0 (pointer, see ChapterLoadStateMachine.cpp)
extern void* DAT_00fcae04;  // the screen/language service object: +0x04 64-byte
                            // current-language name buffer, +0x98 platform
                            // marker, +0xb8/+0xbc the narrow language-name
                            // vector (begin/end)
extern std::int32_t DAT_00879654;  // the selected language's index (-1: none)
extern std::uint32_t DAT_00d6b8a0; // screen-flow flag bits (bit 0: menu shown)
extern std::int32_t DAT_00d6b898;
extern std::int32_t DAT_00d6e5b8;
extern void* DAT_00d46aa8;  // presentation registry whose +0xa0 byte gains flag
                            // bit 2 when a non-English language is committed
extern std::int32_t DAT_00d472dc;  // also in ChapterFlow.cpp
extern void* DAT_00fcadec;  // the chapter registry "this"; also in
                            // ChapterFlow.cpp/BootupFlow.cpp

// 0x005e5e40 (EAX = narrow name, ECX = the language service's +0x04 name
// buffer at the only observed call site; deferred, test-provided): finds
// the wide display text mapped for a language name inside the language
// service's own text map, inserting an empty wide value when the key is
// absent and returning its storage. Only the Update's fill loop calls it
// (once per installed language slot, 16 keys).
extern const wchar_t* lookup_localized_language_text(
    const char* language_name, void* name_buffer_owner);

// The wide " " constant at 0x0081fca8 (Ghidra: _Format_0081fca8): the
// display-text fallback for menu slots beyond the installed language
// count. The game-space constant's contents; tests define the array.
extern const wchar_t kWideBlank_0081fca8[2];// 0x004ccca0 — not reconstructed: an external dependency, same status
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

// "" 0x005e5de0 (FUN_005e5de0; ECX = destination, EAX = source at the
// only observed call — the LanguageSelect confirm path, 0x00fd7920).
// Bounded name copy: no-op for a null source; when strlen(source) is
// below the binary's own 0x3f guard, copies the narrow name (including
// its terminating NUL) into the destination; destination bytes past
// the terminator and every case with a >= 0x3f name are left
// untouched. strcpy-equivalent for the observable destination bytes;
// spelled out to keep the guard and the no-op exact.
void copy_language_name_into(void* destination, const char* source) {
    if (source == nullptr) {
        return;
    }
    const std::size_t length = std::strlen(source);
    if (length < 0x3f) {
        auto* out = static_cast<char*>(destination);
        for (std::size_t i = 0; i <= length; ++i) {
            out[i] = source[i];
        }
    }
}

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
    void* pending_task;        // +0xc: Update's own Task_IdleInputTimer submission
    std::int32_t update_tick_snapshot; // +0x10: tick snapshot Update's state 1 takes
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

    // "" FUN_00fd7920 (reached in the binary through thunk_FUN_00fd7920
    // from exactly one caller: this Update's state-2 timeout branch,
    // 0x00429cb0). Commits the selected language and hops to the
    // FrontEnd chapter. See ScreenTasks.h for the deferred blocks.
    std::int32_t try_confirm_language_selection() {
        if (DAT_00879654 >= 0) { // "" if (-1 < DAT_00879654)
            auto* service = static_cast<std::uint8_t*>(DAT_00fcae04);
            // The exported x86 re-walks the +0xb8/+0xbc vector with an
            // index counter and stops at the DAT_00879654-th element;
            // an in-range index selects exactly that element and an
            // out-of-range one selects nothing. On the game's 32-bit
            // target the elements are 4-byte slots, so the element
            // count is the byte distance >> 2.
            char** begin =
                reinterpret_cast<char**>(*reinterpret_cast<void**>(service + 0xb8));
            char** end =
                reinterpret_cast<char**>(*reinterpret_cast<void**>(service + 0xbc));
            if (DAT_00879654 < static_cast<std::int32_t>(end - begin)) {
                const char* selected = begin[DAT_00879654];
                if (selected != nullptr) {
                    // "" 0x005e5de0 (ECX = dst, EAX = src): bounded
                    // copy of the selected name into the service's own
                    // +0x04 name buffer (the same buffer the
                    // 0x005e5e40 text lookups key against).
                    copy_language_name_into(service + 4, selected);

                    // NOT YET INTEGRATED: FUN_005da670(selected) —
                    // refreshes the game window's caption from the
                    // language-row table at 0x008925b0 (needs the
                    // window object at DAT_00cef694). Its only
                    // observable is the OS window title.
                    //
                    // NOT YET INTEGRATED: sprintf(buf,
                    // "Game/Localised/%s.str", selected) +
                    // thunk_FUN_00ff7f20(buf, buf) — the localized .str
                    // resource hook, the same documented deferred
                    // no-op as the "Game.str" load in
                    // trigger_bootup_flow (BootupFlow.cpp): nothing in
                    // this project's archive/stream stack reads it yet.
                    //
                    // "" The exported x86 compares the selected name
                    // against its own 6-byte "en-GB" (0x0083e694) and
                    // "en-US" (0x0083e68c) constants with unrolled
                    // byte-pair loops (5 chars + NUL, observably
                    // strcmp-equivalent); a name different from BOTH
                    // markers AND a non-null registry pointer at
                    // DAT_00d46aa8 raises that registry's +0xa0 flag
                    // bit 2.
                    if (std::strcmp(selected, "en-GB") != 0 &&
                        std::strcmp(selected, "en-US") != 0 &&
                        DAT_00d46aa8 != nullptr) {
                        *reinterpret_cast<std::uint32_t*>(
                            static_cast<std::uint8_t*>(DAT_00d46aa8) + 0xa0) |= 0x4u;
                    }
                }
            }
        }
        DAT_00d472dc = 8;
        // FUN_00576f30's return value only ever feeds this function's
        // leftover EAX (the same "void callee, a caller reads EAX"
        // pattern BootupFlow.cpp documents for trigger_bootup_flow's
        // own resolve_and_load_chapter call; the leftover is not
        // modeled), so the observable return is the low byte 1.
        resolve_and_load_chapter(DAT_00fcadec, "FrontEnd");
        return 1;
    }

    // "" 0x00429890, vtable slot 3. See ScreenTasks.h for the full
    // state-machine description; deferred blocks are documented in
    // place.
    std::int32_t update() {
        if (state == 0) {
            if (DAT_00cef610 != nullptr &&
                *reinterpret_cast<const std::uint8_t*>(
                    static_cast<const std::uint8_t*>(DAT_00cef610) + 0xb0) == 0) {
                state = 1;
            }
            // The exported x86's state-0 exit still carries
            // DAT_00cef610's own value in EAX (written there by the
            // entry compare); replicated literally, the way
            // ChapterLoadStateMachine.cpp does for its
            // `& 0xffffff00` returns.
            return static_cast<std::int32_t>(
                reinterpret_cast<std::uintptr_t>(DAT_00cef610) &
                0xffffff00u);
        }
        if (state == 1) {
            // The tick snapshot the state-2 timeout is measured
            // against (task+0x10).
            update_tick_snapshot =
                static_cast<std::int32_t>(hp6::runtime::current_tick_value());

            // Construct + submit the same 0x10-byte
            // "Presentation:TaskHandler::Task_IdleInputTimer"
            // enqueue_title_page_screen_task's IsReady constructs
            // (same vtable); storing the submission result at +0xc is
            // what makes this task's own slot-2 able to flag it ready.
            auto* timer = static_cast<IdleInputTimerTask*>(allocate_via_task_handler(
                DAT_00fcadf0, sizeof(IdleInputTimerTask),
                "Presentation:TaskHandler::Task_IdleInputTimer"));
            if (timer != nullptr) {
                timer->ready_flag = 0;
                timer->vtable = kIdleInputTimerTaskVtable;
            }
            auto* submitted =
                static_cast<IdleInputTimerTask*>(submit_task(DAT_00fcadf0, timer));
            pending_task = submitted;
            if (submitted != nullptr) {
                // The exported x86 sets the submitted sub-task's own
                // +0xc (the IdleInputTimer's state field) to the -1
                // "not yet armed" sentinel unconditionally; the guard
                // only covers the never-observed allocation-failure
                // path, as in the sibling enqueue_* functions.
                submitted->state = -1;
            }

            // The fill loop: 16 writes into the shared 0x00cf8168 text
            // table, one per "GUI_LANGUAGEMENU%d" key. In-range slots
            // get the wide display text the service's own map holds for
            // that language (the deferred 0x005e5e40 stand-in); slots
            // beyond the language count get the wide " " constant at
            // 0x0081fca8. The exported x86 zeroes its 0x100-byte key
            // buffer each iteration and sprintf-s the key into it.
            auto* service = static_cast<std::uint8_t*>(DAT_00fcae04);
            char** begin =
                reinterpret_cast<char**>(*reinterpret_cast<void**>(service + 0xb8));
            char** end =
                reinterpret_cast<char**>(*reinterpret_cast<void**>(service + 0xbc));
            for (std::int32_t i = 0; i < 0x10; ++i) {
                char key[0x100];
                // sprintf in the exported x86; snprintf here only
                // sidesteps MSVC's sprintf deprecation warning.
                std::snprintf(key, sizeof(key), "GUI_LANGUAGEMENU%d", i);
                const wchar_t* text = kWideBlank_0081fca8;
                if (i < static_cast<std::int32_t>(end - begin)) {
                    // The exported x86 finds element i by walking the
                    // vector with a counter; in range, that is exactly
                    // `begin[i]` (narrow char* entries).
                    text = lookup_localized_language_text(begin[i], service + 4);
                }
                update_localized_text(0x00cf8168u, key, text);
            }

            // NOT YET INTEGRATED: the widget-registry resync that
            // follows the fill in the exported x86:
            // thunk_FUN_0100e910 (current-language row getter), its
            // name-vs-list check (FUN_005e5f30), then an indexed walk
            // over the same vector writing each matching UI widget's
            // +0xc8 field with that language's index (FUN_00435110
            // finds the widgets in the hash table at 0x00d260dc by
            // name and verifies the DAT_0089c014 type tag), plus
            // FUN_00416f70's follow-up call on the plain name match.
            // None of those widgets (or that hash table) exists in
            // this project yet; the map writes above are the state's
            // surviving observable.
            state = 2;
            // The exported x86's state-1 exit carries `this` in EAX;
            // replicated literally (see ScreenTasks.h).
            return static_cast<std::int32_t>(
                reinterpret_cast<std::uintptr_t>(this) & 0xffffff00u);
        }
        if (state == 2) {
            auto* service = static_cast<std::uint8_t*>(DAT_00fcae04);
            char** begin =
                reinterpret_cast<char**>(*reinterpret_cast<void**>(service + 0xb8));
            char** end =
                reinterpret_cast<char**>(*reinterpret_cast<void**>(service + 0xbc));
            // The exported x86's own condition is
            // `((byte_distance & ~3) == 4) || strstr(+0x98, "WBSUB")`:
            // on the 32-bit target that byte-distance form is exactly
            // "one installed language", which is what any vector can
            // satisfy it with, so it is written here as the element
            // count it reduces to.
            if (static_cast<std::int32_t>(end - begin) == 1 ||
                std::strstr(
                    reinterpret_cast<const char*>(service + 0x98),
                    "WBSUB") != nullptr) {
                DAT_00879654 = 0;
            }
            DAT_00d6b8a0 |= 1u;
            DAT_00d6b898 = 0;
            DAT_00d6e5b8 = 0;

            // NOT YET INTEGRATED: the 15000-tick idle-reselection scan.
            // Once DAT_00d644f8/3 - <the IdleInputTimer sub-task's own
            // +0x8 cached tick>/3 exceeds 15000, the exported x86
            // re-reads the current-process language row
            // (thunk_FUN_0100e910), checks that row's name against the
            // service's language list (FUN_005e5f30), and re-derives
            // DAT_00879654 from a FUN_005e5f00-driven indexed walk over
            // the vector (or resets it to 0 when the current row's
            // language is not in the list). That scan's only observable
            // is DAT_00879654, which the commit path consumes; tests
            // that reach the commit set DAT_00879654 directly.
            const std::int32_t elapsed =
                static_cast<std::int32_t>(hp6::runtime::current_tick_value()) /
                    3 -
                update_tick_snapshot / 3;
            if (elapsed > 0x14d) {
                if (try_confirm_language_selection() != 0) {
                    // The exported x86 blends the confirm thunk's own
                    // leftover EAX into the high bits; the low byte 1 is
                    // the confirm's guaranteed tail (see
                    // try_confirm_language_selection's tail comment).
                    return 1;
                }
                // Unreachable given the confirm's unconditional tail
                // (its low byte is always 1); left structured like the
                // binary's own branch.
            }
            // The exported x86's not-yet-committed exit carries the
            // elapsed value itself in EAX; replicated literally (see
            // ScreenTasks.h).
            return static_cast<std::int32_t>(
                static_cast<std::uint32_t>(elapsed) & 0xffffff00u);
        }
        // States beyond 2 are unreachable through the transitions
        // above. The exported x86's exit computes (state - 2) in EAX;
        // replicated literally.
        return static_cast<std::int32_t>(
            (static_cast<std::uint32_t>(state) - 2u) & 0xffffff00u);
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

// All four task-object vtable slots are reconstructed for this task
// (slot 3, "Update" (0x00429890), was until the 19 oct 2026
// integration left as the shared no-op stub — see
// ScreenLanguageSelectTask in ScreenTasks.h and
// update_language_select_screen_task below).
void* kLanguageSelectScreenTaskVtable[4] = {
    member_function_address(&ScreenLanguageSelectTask::destroy),
    member_function_address(&ScreenLanguageSelectTask::query_ready),
    member_function_address(&ScreenLanguageSelectTask::mark_pending_task_ready_if_enqueued),
    member_function_address(&ScreenLanguageSelectTask::update),
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

std::int32_t update_language_select_screen_task(void* task_ptr) {
    return static_cast<ScreenLanguageSelectTask*>(task_ptr)->update();
}

} // namespace hp6::presentation
