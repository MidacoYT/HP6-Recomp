#include "presentation/BootupFlow.h"
#include "presentation/ChapterFlow.h"
#include "presentation/CommandDispatch.h"
#include "presentation/TaskSystem.h"

#include <cstdint>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern void* DAT_00fcadf0;     // PresentationTaskHandler singleton
extern void* PTR_LAB_00828360[4]; // Task_BootupFlowControl's real vtable
extern std::uint8_t DAT_008663d5; // quality-tier flag; also in DeviceReset.cpp
extern void* DAT_00fcadec;     // chapter registry; also in ChapterFlow.cpp

namespace hp6::presentation {

std::int32_t handle_trigger_bootup_flow(const char* /*args*/) {
    void* task = allocate_via_task_handler(
        DAT_00fcadf0, 8, "Presentation:TaskHandler::Task_BootupFlowControl");
    if (task != nullptr) {
        static_cast<std::uint8_t*>(task)[4] = 0;
        *reinterpret_cast<void**>(task) = PTR_LAB_00828360;
        submit_task(DAT_00fcadf0, task);
    } else {
        submit_task(DAT_00fcadf0, nullptr);
    }
    return 1;
}

std::uint32_t trigger_bootup_flow(void* /*task*/) {
    if (DAT_008663d5 == 0) {
        // NOT YET INTEGRATED: loads "SSGShaders" (FUN_00653a40/ef0) and
        // "Shaders.str" (thunk_FUN_01004f90) — a documented deferred
        // no-op block. Neither this project's render path nor its
        // chapter-flow path depends on it yet.
    }

    // NOT YET INTEGRATED: thunk_FUN_00ff7f20("Game.str") — loads the
    // game's main data archive. Its return value only ever reaches the
    // low byte of this function's own return, which no caller found so
    // far reads (WinMain discards the result). Deferred as a documented
    // no-op returning 0.
    const std::uint32_t game_str_result = 0;

    // The exported x86 compares the literal "Global" against itself
    // (always equal) before dispatching — reproduced here directly as
    // an unconditional dispatch, since the comparison can never be
    // false.
    dispatch_presentation_command("TriggerBootupFlow");

    // resolve_and_load_chapter's exported x86 counterpart (FUN_00576f30)
    // is declared void by Ghidra, yet this caller reads its leftover
    // EAX afterward (the same "void but a real caller reads AL/EAX"
    // pattern already documented on dispatch_presentation_command) —
    // that leftover value is not modeled here, since nothing this
    // project has traced ever reads WinMain's own return from
    // trigger_bootup_flow.
    resolve_and_load_chapter(DAT_00fcadec, "Global");

    return game_str_result & 0xffu;
}

} // namespace hp6::presentation
