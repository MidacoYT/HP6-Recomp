#include <cstdint>

extern void service_heap_lock_005dd290(void* lock);
extern void service_heap_unlock_005dd2a0(void* lock);
extern void* service_heap_allocate_aligned_core_005de780(
    void* heap, std::uint32_t size, std::uint32_t alignment,
    std::uint32_t region, std::uint32_t flags);

// 0x005DE700 jumps to 0x00FF91A0..0x00FF9219. The original holds
// heap+0x4E4 around the five-argument call to 0x005DE780.
void* service_heap_allocate_aligned_locked_005de700(
    void* heap, std::uint32_t size, std::uint32_t alignment,
    std::uint32_t region, std::uint32_t flags) {
    void* lock = *reinterpret_cast<void**>(
        static_cast<std::uint8_t*>(heap) + 0x4e4);
    if (lock != nullptr) service_heap_lock_005dd290(lock);
    void* payload = service_heap_allocate_aligned_core_005de780(
        heap, size, alignment, region, flags);
    if (lock != nullptr) service_heap_unlock_005dd2a0(lock);
    return payload;
}
