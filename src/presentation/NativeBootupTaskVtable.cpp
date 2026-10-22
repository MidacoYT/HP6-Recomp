// Relocated 0x00828360 and 0x0083d174 from the unprotected HP6 x86 PE.
// SHA-256: 41cbb7ad4b08f6edeb5c2a06aea037d1c0634e9b71df093cc9a26b2287916e10
#include "presentation/ScreenTasks.h"
#include "platform/HandleRegistry.h"

#include <cstdint>

extern "C" int __cdecl _purecall();

namespace {

struct BootupFlowTask {
    void* vtable;
    std::uint8_t ready;
    std::uint8_t padding[3];

    void* destroy(std::uint32_t flags);
};

template <typename Member>
void* member_address(Member member) {
    union {
        Member method;
        void* address;
    } conversion{};
    conversion.method = member;
    return conversion.address;
}

std::int32_t task_ready() { return 1; } // 0x0045df40
void unused_slot() {}                  // 0x00405a30

// 0x0083d174, the four slots this task can see after destruction.
void* destroyed_task_vtable[4] = {
    member_address(&BootupFlowTask::destroy),
    reinterpret_cast<void*>(&_purecall),
    reinterpret_cast<void*>(&unused_slot),
    reinterpret_cast<void*>(&_purecall),
};

void* BootupFlowTask::destroy(std::uint32_t flags) {
    vtable = destroyed_task_vtable;
    if ((flags & 1u) != 0) {
        hp6::platform::release_through_service(this);
    }
    return this;
}

} // namespace

// 0x00828360: destroying destructor, IsReady, no-op, Update.
void* PTR_LAB_00828360[4] = {
    member_address(&BootupFlowTask::destroy),
    reinterpret_cast<void*>(&task_ready),
    reinterpret_cast<void*>(&unused_slot),
    reinterpret_cast<void*>(&hp6::presentation::begin_language_selection),
};
