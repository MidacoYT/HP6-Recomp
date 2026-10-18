#pragma once
#include <cstdint>

namespace hp6::presentation {

// 0x0048c970 (the "TriggerBootupFlow" command handler, table entry #73 —
// see architecture/presentation_commands.csv). Allocates an 8-byte task
// object through PresentationTaskHandler's nested allocator (the same
// pattern already factored out as allocate_via_task_handler), tags it
// "Presentation:TaskHandler::Task_BootupFlowControl", gives it the real
// vtable at PTR_LAB_00828360 (an extern the final binary must supply,
// like every other not-yet-identified vtable in this project — its
// slot 1, "IsReady", is called immediately by submit_task), and submits
// it to the same PresentationTaskHandler (DAT_00fcadf0). On allocation
// failure, still submits a null task (matching the exported x86's own
// fallback exactly). Always returns 1 in its low byte.
std::int32_t handle_trigger_bootup_flow(const char* args);

// 0x00ff7fa0 ("TriggerBootupFlow" in the exported x86's own string
// table — this is the function that, once registered, makes dispatching
// that command name actually do something; reached from WinMain via the
// thunk at 0x005d6cb0, called once, directly, right after the engine
// bootstrap). When DAT_008663d5 (the quality-tier flag, already set by
// hp6::render::update_quality_tier_flag) is zero, loads "SSGShaders" and
// "Shaders.str" — a documented deferred no-op block here, since neither
// this project's rendering path nor its chapter-flow path depends on it
// yet. Always loads "Game.str" (FUN_00ff7f20, deferred), then
// dispatches the "TriggerBootupFlow" command itself (which registers
// Task_BootupFlowControl, see handle_trigger_bootup_flow above) and
// resolves/loads the chapter named "Global" through
// resolve_and_load_chapter (already integrated — confirmed, see
// architecture/startup_path.md, to leave DAT_00d472dc untouched since
// "Global" matches neither chapter-name table).
std::uint32_t trigger_bootup_flow(void* task);

} // namespace hp6::presentation
