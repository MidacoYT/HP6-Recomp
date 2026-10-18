// Local behavior checks for the presentation task system (0x005dc750,
// the shared named allocator; 0x005d2b30, the task-queue append). See
// architecture/startup_path.md for how these fit into the
// WinMain -> TriggerBootupFlow -> Task_Bootup chain.
#include "presentation/TaskSystem.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#define HP6_STDCALL __stdcall
#define HP6_CDECL __cdecl
#else
#define HP6_STDCALL
#define HP6_CDECL
#endif

// ---- hp6::platform::service_holder's dependencies (shared lazy-init
// block; see tests/handle_registry_smoke.cpp for the same pattern) ----
void* PTR_FUN_0083e55c = nullptr;
void* PTR_FUN_0083e56c = nullptr;
void* DAT_00fcdd74 = nullptr;
void* DAT_00fcdd78 = nullptr;
void** DAT_00cef6bc = nullptr;
std::uint32_t DAT_00fd1254 = 0;
void cleanup_00711e60() {}
// Pulled in transitively through HandleRegistry.obj (linking any symbol
// from a .obj requires every undefined symbol that whole .obj needs);
// unused by this test beyond satisfying the linker.
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

// ---- allocate_tagged's fake named-allocation service ------------------
int named_allocate_calls = 0;
std::int32_t last_named_size = 0;
const char* last_named_tag = nullptr;
std::uint8_t named_arena[64];

struct TagDescriptorView {
    std::int32_t flag;
    const char* name;
    std::int32_t reserved;
};

void* HP6_CDECL fake_named_allocate(std::int32_t size, const void* descriptor) {
    ++named_allocate_calls;
    last_named_size = size;
    last_named_tag = static_cast<const TagDescriptorView*>(descriptor)->name;
    return named_arena;
}

// ---- submit_task's fake nested allocator + fake task object -----------
int node_allocate_calls = 0;
std::uint8_t node_arena[3][sizeof(void*) * 3];
int next_node = 0;

// Thiscall and stdcall are ABI-compatible here: both are callee-clean
// for explicit stack arguments, and the (unused) `this` travels via
// ECX either way, matching tests/pool_list_smoke.cpp's stdcall-declared
// fake for a thiscall-typed vtable slot — dropping the `self` parameter
// entirely (rather than declaring it explicitly) keeps the explicit
// argument count, and so the stack layout, identical to the real call.
void* HP6_STDCALL fake_node_allocate(std::int32_t size, const void* descriptor) {
    (void)size;
    (void)descriptor;
    ++node_allocate_calls;
    void* slot = node_arena[next_node];
    ++next_node;
    return slot;
}
void* node_allocator_vtable[1] = {reinterpret_cast<void*>(&fake_node_allocate)};
struct FakeNodeAllocator {
    void* vtable = node_allocator_vtable;
};
FakeNodeAllocator fake_node_allocator;

int task_query_calls = 0;
bool task_query_result = false;
std::int32_t HP6_STDCALL fake_task_query() {
    ++task_query_calls;
    return task_query_result ? 1 : 0;
}
void* task_vtable[2] = {nullptr, reinterpret_cast<void*>(&fake_task_query)};
struct FakeTask {
    void* vtable = task_vtable;
    std::uint8_t ready_flag = 0;
};

struct FakeTaskHandler {
    std::uint8_t pad[8] = {};
    void* allocator = &fake_node_allocator;
    void* queue_head = nullptr;
};

} // namespace

int main() {
    using namespace hp6::presentation;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // ---- allocate_tagged (005dc750) ---------------------------------------
    PTR_FUN_0083e56c = reinterpret_cast<void*>(&fake_named_allocate);
    void* allocated = allocate_tagged(0x164, "GOF::GameServicesCreate::FMVDataManager");
    expect("named_allocate_called_once", named_allocate_calls == 1);
    expect("named_allocate_size", last_named_size == 0x164);
    expect("named_allocate_tag",
           last_named_tag != nullptr &&
               std::strcmp(last_named_tag, "GOF::GameServicesCreate::FMVDataManager") == 0);
    expect("named_allocate_result", allocated == named_arena);
    expect("holder_lazy_init_ran", (DAT_00fd1254 & 1u) != 0);

    // ---- submit_task (005d2b30): empty queue, task not yet ready ---------
    FakeTaskHandler handler;
    FakeTask task_a;
    task_query_result = false;
    void* result_a = submit_task(&handler, &task_a);
    expect("submit_returns_task", result_a == &task_a);
    expect("submit_node_allocated_once", node_allocate_calls == 1);
    expect("submit_queue_head_set", handler.queue_head != nullptr);
    expect("submit_marks_not_ready", task_a.ready_flag == 1);

    // ---- submit_task: second task appends to the tail, already ready ----
    FakeTask task_b;
    task_query_result = true;
    void* result_b = submit_task(&handler, &task_b);
    expect("submit_second_returns_task", result_b == &task_b);
    expect("submit_second_node_allocated", node_allocate_calls == 2);
    expect("submit_second_leaves_ready_flag", task_b.ready_flag == 0);

    // ---- submit_task: null task is a no-op --------------------------------
    expect("submit_null_task", submit_task(&handler, nullptr) == nullptr);
    expect("submit_null_task_no_alloc", node_allocate_calls == 2);

    if (failures != 0) {
        std::printf("task_system_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("task_system_smoke: all checks passed\n");
    return 0;
}
