#include <cstdint>

extern void service_heap_lock_005dd290(void* lock);
extern void service_heap_unlock_005dd2a0(void* lock);
extern void service_heap_free_payload_005de580(void* heap, void* payload);

void service_heap_release_region_005de540(void* heap, void* region) {
    // 0x005DE540 -> 0x00FF8FE0..0x00FF9015. EAX carries heap and the
    // single stack argument carries region. The thunk locks heap+0x4E4,
    // forwards the region to 0x005DE580, then releases the same lock.
    void* lock = *reinterpret_cast<void**>(
        static_cast<std::uint8_t*>(heap) + 0x4e4);
    if (lock != nullptr) service_heap_lock_005dd290(lock);
    service_heap_free_payload_005de580(heap, region);
    if (lock != nullptr) service_heap_unlock_005dd2a0(lock);
}
