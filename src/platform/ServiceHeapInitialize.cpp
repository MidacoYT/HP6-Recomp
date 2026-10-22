#include "platform/ServiceHeapInitialize.h"

#include <cstdint>
#include <cstring>
#include <windows.h>

extern void service_heap_enable_lock_005dd830_0(void* heap);
extern std::uint32_t service_heap_allocate_00ff9850(
    void* heap, std::uint32_t size, std::uint8_t mode,
    std::uint32_t, std::uint32_t, std::uint32_t);
extern void service_heap_lock_005dd290(void* lock);
extern void service_heap_unlock_005dd2a0(void* lock);
extern void* DAT_00cef724;

namespace {
template <typename T>
T& at(std::uint8_t* base, std::uintptr_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}

struct CallbackNode {
    std::uint32_t (*callback)(void*, std::uint32_t, void*);
    void* context;
    CallbackNode* next;
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(CallbackNode) == 12);
#endif
} // namespace

std::uint32_t service_heap_initialize_005dd4c0(
    void* heap_ptr, std::uint32_t request, std::uint32_t size, std::uint8_t mode) {
    // 0x005DD4C0..0x005DD6A1; stack argument 1 controls whether to call
    // 0x00FF9850 even when stack argument 2 is zero.
    auto* heap = static_cast<std::uint8_t*>(heap_ptr);
    if (heap[0] == 0) {
        heap[0] = 1;
        service_heap_enable_lock_005dd830_0(heap);
        void* lock = at<void*>(heap, 0x4e4);
        if (lock != nullptr) service_heap_lock_005dd290(lock);

        at<std::uint32_t>(heap, 4) = 0x40;
        std::memset(heap + 8, 0, 0x28);
        std::memset(heap + 0x30, 0, 0x400);
        for (std::uint32_t index = 0; index < 0x7f; ++index) {
            auto* entry = heap + 0x30 + index * 8;
            at<void*>(entry, 8) = entry;
            at<void*>(entry, 12) = entry;
        }
        std::memset(heap + 0x430, 0, 0x10);
        at<void*>(heap, 0x440) = heap + 0x30;
        std::memset(heap + 0x444, 0, 0x24);
        at<void*>(heap, 0x460) = heap + 0x448;
        at<void*>(heap, 0x464) = heap + 0x448;
        at<std::uint32_t>(heap, 4) = (at<std::uint32_t>(heap, 4) & 1) | 0x48;
        at<std::uint32_t>(heap, 0x468) = 0;
        heap[0x46c] = 0;
        at<std::uint32_t>(heap, 0x488) = 0;
        at<std::uint32_t>(heap, 0x48c) = 0;
        at<std::uint32_t>(heap, 0x490) = 0x10000;
        at<std::uint32_t>(heap, 0x494) = 0x20000;
        heap[0x498] = 0;
        std::memset(heap + 0x49c, 0, 0x10);
        at<void*>(heap, 0x4a4) = heap + 0x49c;
        at<void*>(heap, 0x4a8) = heap + 0x49c;
        at<std::uint32_t>(heap, 0x4c8) = 0x40000;
        at<std::uint32_t>(heap, 0x4cc) = 0x10000;
        at<void*>(heap, 0x4d0) = at<void*>(heap, 0x440);
        SYSTEM_INFO system_info{};
        GetSystemInfo(&system_info);
        at<std::uint32_t>(heap, 0x4d4) = system_info.dwPageSize;
        if (lock != nullptr) service_heap_unlock_005dd2a0(lock);
    }

    std::uint32_t result = 0;
    if (request != 0 || size != 0)
        result = service_heap_allocate_00ff9850(heap, size, mode, 0, 0, 0);
    if (heap[0x514] == 0) {
        heap[0x514] = 1;
        for (auto* node = static_cast<CallbackNode*>(DAT_00cef724);
             node != nullptr; node = node->next) {
            result = node->callback(heap, 1, node->context);
        }
    }
    return (result & ~0xffu) | 1u;
}
