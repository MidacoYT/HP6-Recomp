#include "platform/ServiceHeapTeardown.h"
#include "platform/ServiceHeapLeafs.h"

#include <cstdint>

// Direct callees in the unprotected PE. They are defined in separate
// reconstruction units so the caller's control flow remains auditable.
extern void service_heap_lock_005dd290(void* lock);
extern void service_heap_prepare_release_after_flag_005dec90(void* heap);
extern void service_heap_free_payload_005de580(void* heap, void* payload);
extern void service_heap_detach_node_005def40(void* node);
extern void service_heap_destroy_node_005df520(void* node, void* heap, std::uint8_t flag);
extern void service_heap_unlock_005dd2a0(void* lock);
extern void service_heap_free_lock_005dd280(void* lock);
extern void* DAT_00cef724;

namespace {

template <typename T>
T& at(std::uint8_t* base, std::uintptr_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}

struct CallbackNode {
    void (*callback)(void* heap, std::uint32_t mode, void* context);
    void* context;
    CallbackNode* next;
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(CallbackNode) == 12);
#endif

} // namespace

std::uint32_t teardown_service_heap_005dd6b0(void* heap_ptr) {
    // 0x005DD6B0..0x005DD822 in the unprotected HP6 PE.
    auto* heap = static_cast<std::uint8_t*>(heap_ptr);
    if (at<std::uint8_t>(heap, 0x514) == 1) {
        at<std::uint8_t>(heap, 0x514) = 0;
        for (auto* node = static_cast<CallbackNode*>(DAT_00cef724);
             node != nullptr; node = node->next) {
            node->callback(heap, 0, node->context);
        }
    }

    void* lock = at<void*>(heap, 0x4e4);
    if (lock != nullptr) {
        service_heap_lock_005dd290(lock);
    }

    if (heap[0] != 0) {
        heap[0] = 0;
        if (hp6::platform::heap_header_flag_005db490(heap) != 0) {
            // The caller has just tested word(heap+4) & 1, so the zero-word
            // initialization branch inside 0x005DEC90 is unreachable here.
            service_heap_prepare_release_after_flag_005dec90(heap);
        }

        auto* first_sentinel = heap + 0x49c;
        while (at<void*>(heap, 0x4a8) != first_sentinel) {
            void* node = at<void*>(heap, 0x4a8);
            auto* base = hp6::platform::heap_relative_base_005db470(node);
            auto* payload = hp6::platform::heap_node_payload_005db440(base);
            service_heap_free_payload_005de580(heap, payload);
        }

        auto* second_sentinel = heap + 0x448;
        while (at<void*>(heap, 0x464) != second_sentinel) {
            void* node = at<void*>(heap, 0x464);
            service_heap_detach_node_005def40(node);
            service_heap_destroy_node_005df520(node, heap, 1);
        }

        at<void*>(heap, 0x440) = hp6::platform::heap_control_at_30_005db480(heap);
        at<std::uint32_t>(heap, 0x468) = 0;
        at<std::uint8_t>(heap, 0x46c) = 0;
    }

    // The x86 body re-reads this slot after the node cleanup; callees may
    // have changed it since the initial lock operation.
    if (void* final_lock = at<void*>(heap, 0x4e4); final_lock != nullptr) {
        at<void*>(heap, 0x4e4) = nullptr;
        service_heap_unlock_005dd2a0(final_lock);
        service_heap_free_lock_005dd280(final_lock);
    }
    return 1;
}
