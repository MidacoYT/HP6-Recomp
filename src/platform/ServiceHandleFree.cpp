#include <cstdint>

extern void* DAT_00cef6b0;
extern void service_heap_lock_005dd290(void* lock);
extern void service_heap_unlock_005dd2a0(void* lock);
extern void service_heap_free_payload_005de580(void* heap, void* payload);

// 0x005DB8D0..0x005DB912 in the unprotected x86 PE: service vtable slot 3.
// The first stack argument (the block) is tested before accessing the heap.
void service_handle_free_005db8d0(void* block, std::uint32_t) {
    if (block == nullptr) return;
    auto* heap = static_cast<std::uint8_t*>(DAT_00cef6b0);
    void* lock = *reinterpret_cast<void**>(heap + 0x4e4);
    if (lock != nullptr) service_heap_lock_005dd290(lock);
    service_heap_free_payload_005de580(heap, block);
    if (lock != nullptr) service_heap_unlock_005dd2a0(lock);
}
