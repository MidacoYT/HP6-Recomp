#include "presentation/TaskSystem.h"
#include "platform/HandleRegistry.h"

#include <cstdint>

// Shared with src/platform/HandleRegistry.cpp's service_holder(), which
// performs the lazy init both globals depend on.
extern void* DAT_00fcdd78;

namespace hp6::presentation {
namespace {

void* vtable_of(void* object) {
    return *reinterpret_cast<void* const*>(object);
}

#ifdef _MSC_VER
using NamedAllocateCall = void* (__cdecl*)(std::int32_t, const void*);
using SubsystemAllocateCall = void* (__thiscall*)(void*, std::int32_t, const void*);
using TaskQueryCall = std::int32_t (__thiscall*)(void*);
#else
using NamedAllocateCall = void* (*)(std::int32_t, const void*);
using SubsystemAllocateCall = void* (*)(void*, std::int32_t, const void*);
using TaskQueryCall = std::int32_t (*)(void*);
#endif

struct TagDescriptor {
    std::int32_t flag;
    const char* name;
    std::int32_t reserved;
};

struct TaskQueueNode {
    TaskQueueNode* prev;
    TaskQueueNode* next;
    void* task;
};

} // namespace

void* allocate_tagged(std::int32_t size, const char* name) {
    hp6::platform::service_holder();
    const TagDescriptor descriptor{1, name, 0};
    // DAT_00fcdd78 holds the address of PTR_FUN_0083e56c, whose own
    // content is the real function pointer — one more dereference than
    // service_slot() needs, since this global holds a plain function
    // pointer rather than a vtable base.
    auto allocate = *reinterpret_cast<NamedAllocateCall*>(DAT_00fcdd78);
    return allocate(size, &descriptor);
}

void* allocate_via_task_handler(void* task_handler, std::int32_t size, const char* tag) {
    void* allocator = *reinterpret_cast<void**>(static_cast<std::uint8_t*>(task_handler) + 8);
    auto allocate = *reinterpret_cast<SubsystemAllocateCall const*>(vtable_of(allocator));
    const TagDescriptor descriptor{1, tag, 0};
    return allocate(allocator, size, &descriptor);
}

void* submit_task(void* task_handler, void* task) {
    if (task == nullptr) {
        return nullptr;
    }

    auto* header = static_cast<std::uint8_t*>(task_handler);
    auto& queue_head = *reinterpret_cast<TaskQueueNode**>(header + 0xc);

    TaskQueueNode* node = nullptr;
    if (queue_head == nullptr) {
        node = static_cast<TaskQueueNode*>(
            allocate_via_task_handler(task_handler, 0xc, "Presentation:TaskHandler::STaskItem"));
        if (node != nullptr) {
            node->prev = nullptr;
            node->next = nullptr;
            node->task = task;
        }
        queue_head = node;
    } else {
        TaskQueueNode* tail = queue_head;
        while (tail->next != nullptr) {
            tail = tail->next;
        }
        node = static_cast<TaskQueueNode*>(
            allocate_via_task_handler(task_handler, 0xc, "Presentation:TaskHandler::STaskItem"));
        if (node != nullptr) {
            node->prev = tail;
            node->next = nullptr;
            node->task = task;
            tail->next = node;
        } else {
            tail->next = nullptr;
        }
    }

    if (node != nullptr) {
        auto query = *reinterpret_cast<TaskQueryCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(node->task)) + 4);
        if (query(node->task) == 0) {
            *(static_cast<std::uint8_t*>(node->task) + 4) = 1;
        }
    }
    return task;
}

} // namespace hp6::presentation
