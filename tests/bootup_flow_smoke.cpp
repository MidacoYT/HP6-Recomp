// Local behavior checks for the "TriggerBootupFlow" command handler
// (0x0048c970, table entry #73) and FUN_00ff7fa0 (the function that
// dispatches that command, reached from WinMain — see
// architecture/startup_path.md). Game-space symbols get test-local
// definitions, the way the other smoke tests already do.
#include "presentation/BootupFlow.h"
#include "presentation/ChapterFlow.h"
#include "presentation/CommandDispatch.h"
#include "presentation/TaskSystem.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#define HP6_STDCALL __stdcall
#else
#define HP6_STDCALL
#endif

// ---- BootupFlow.cpp's own extern dependencies -----------------------------
std::uint8_t DAT_008663d5 = 1; // quality tier already resolved; skip the
                               // deferred SSGShaders/Shaders.str block
void* DAT_00fcadec = nullptr;

// ---- ChapterFlow.cpp's own extern dependencies ----------------------------
std::uint32_t DAT_00fd1b14 = 0;
void* DAT_00fcd530 = nullptr;
std::uint8_t* DAT_00fcd534 = nullptr;
std::int32_t DAT_00d472dc = -1;
std::uint8_t DAT_0087e014 = 1;
std::uint32_t DAT_00fd178c = 0;
void* DAT_00fd1718 = nullptr;
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

// ---- handle_trigger_bootup_flow's own extern dependencies -----------------
void* DAT_00fcadf0 = nullptr; // PresentationTaskHandler
void* PTR_LAB_00828360 = nullptr; // Task_BootupFlowControl's real vtable

// ---- hp6::platform::service_holder dependencies (pulled in transitively
// through allocate_tagged, not exercised directly by this test but
// required to link) ----------------------------------------------------
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

// ---- fake nested allocator shared by allocate_via_task_handler /
// submit_task, matching tests/task_system_smoke.cpp's own pattern -------
int node_allocate_calls = 0;
std::uint8_t node_arena[4][0x20];
int next_node = 0;
void* HP6_STDCALL fake_node_allocate(std::int32_t, const void*) {
    ++node_allocate_calls;
    void* slot = node_arena[next_node];
    ++next_node;
    return slot;
}
void* node_allocator_vtable[1] = {reinterpret_cast<void*>(&fake_node_allocate)};
struct FakeNodeAllocator { void* vtable = node_allocator_vtable; };
FakeNodeAllocator fake_node_allocator;

struct FakeTaskHandler {
    std::uint8_t pad[8] = {};
    void* allocator = &fake_node_allocator;
    void* queue_head = nullptr;
};

int bootup_task_ready_calls = 0;
std::int32_t HP6_STDCALL fake_bootup_task_ready() {
    ++bootup_task_ready_calls;
    return 0; // not ready: matches submit_task marking byte+4 = 1
}
void* bootup_task_vtable[2] = {nullptr, reinterpret_cast<void*>(&fake_bootup_task_ready)};

std::int32_t unused_command_handler(const char*) { return 0; }
} // namespace

#define HP6_X5(x) x, x, x, x, x
#define HP6_X10(x) HP6_X5(x), HP6_X5(x)

using hp6::presentation::CommandTableEntry;
const CommandTableEntry kUnusedCommand{"", &unused_command_handler};
const CommandTableEntry kTriggerBootupFlow{"TriggerBootupFlow",
                                           &hp6::presentation::handle_trigger_bootup_flow};
extern const CommandTableEntry* const PTR_PTR_00876ab0[93] = {
    &kTriggerBootupFlow,
    HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand),
    HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand),
    HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand),
    &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand,
    &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand,
    &kUnusedCommand, &kUnusedCommand,
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

    // ---- handle_trigger_bootup_flow (0048c970) -----------------------------
    FakeTaskHandler handler;
    DAT_00fcadf0 = &handler;
    PTR_LAB_00828360 = bootup_task_vtable;

    const std::int32_t handle_result = handle_trigger_bootup_flow(nullptr);
    expect("handler_returns_1", handle_result == 1);
    expect("handler_allocated_task_and_node", node_allocate_calls == 2);
    expect("handler_queue_populated", handler.queue_head != nullptr);
    expect("handler_queried_readiness", bootup_task_ready_calls == 1);

    auto* task = static_cast<std::uint8_t*>(node_arena[0]);
    expect("handler_set_vtable", *reinterpret_cast<void**>(task) == bootup_task_vtable);

    // ---- trigger_bootup_flow (00ff7fa0) -------------------------------------
    // Dispatching "TriggerBootupFlow" re-enters handle_trigger_bootup_flow
    // for real (the table above wires the real handler, not a capturing
    // stub), so the effect to check is a second task queued, not a call
    // count on a separate fake.
    DAT_00d472dc = -1;
    bootup_task_ready_calls = 0;
    trigger_bootup_flow(nullptr);
    expect("trigger_queues_second_bootup_task", bootup_task_ready_calls == 1);
    expect("trigger_leaves_chapter_untouched_for_global", DAT_00d472dc == -1);

    if (failures != 0) {
        std::printf("bootup_flow_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("bootup_flow_smoke: all checks passed\n");
    return 0;
}
