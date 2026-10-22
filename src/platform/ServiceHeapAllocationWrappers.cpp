#include <cstdint>

extern void service_heap_lock_005dd290(void* lock);
extern void service_heap_unlock_005dd2a0(void* lock);
extern void* service_heap_allocate_core_005dde30(
    void* heap, std::uint32_t size, std::uint32_t flags);

namespace {
void* heap_lock(void* heap) {
    return *reinterpret_cast<void**>(static_cast<std::uint8_t*>(heap) + 0x4e4);
}
} // namespace

// 0x005DDDB0..0x005DDE1F: lock around 0x005DDE30, preserving its result.
void* service_heap_allocate_locked_005dddb0(
    void* heap, std::uint32_t size, std::uint32_t flags) {
    void* lock = heap_lock(heap);
    if (lock != nullptr) service_heap_lock_005dd290(lock);
    void* payload = service_heap_allocate_core_005dde30(heap, size, flags);
    if (lock != nullptr) service_heap_unlock_005dd2a0(lock);
    return payload;
}

// 0x005DEDE0 jumps to 0x0100EC00. This is the usable offset to the final
// context-pointer word in a service heap allocation, or zero if invalid.
std::uint32_t service_heap_usable_end_offset_005dede0(
    void* heap, void* payload) {
    void* lock = heap_lock(heap);
    if (lock != nullptr) service_heap_lock_005dd290(lock);
    std::uint32_t result = 0;
    if (payload != nullptr) {
        const auto* bytes = static_cast<const std::uint8_t*>(payload);
        const std::uint32_t header =
            *reinterpret_cast<const std::uint32_t*>(bytes - 4);
        const std::uint32_t length = header & 0x7ffffff8u;
        if ((header & 2u) != 0) {
            result = length - 8u;
        } else if ((bytes[length - 4u] & 1u) != 0) {
            result = length - 4u;
        }
    }
    if (lock != nullptr) service_heap_unlock_005dd2a0(lock);
    return result;
}
