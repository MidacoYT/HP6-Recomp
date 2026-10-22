#include <cstdint>

extern void* DAT_00cef710;
extern const char* service_context_registry_name_005db380(const char* name);
extern void* service_heap_allocate_locked_005dddb0(
    void* heap, std::uint32_t size, std::uint32_t flags);
extern void* service_heap_allocate_aligned_locked_005de700(
    void* heap, std::uint32_t size, std::uint32_t alignment,
    std::uint32_t region, std::uint32_t flags);
extern std::uint32_t service_heap_usable_end_offset_005dede0(
    void* heap, void* payload);

// 0x005DBC50 jumps to 0x00FF89C0..0x00FF8A36 in the verified PE.
// EAX=name, EDI=heap, ECX=alignment; size, region and flags are on the stack.
void* service_heap_allocate_core_005dbc50(
    void* heap, const char* name, std::uint32_t alignment,
    std::uint32_t size, std::uint32_t region, std::uint32_t flags) {
    const char* context = DAT_00cef710 != nullptr
        ? service_context_registry_name_005db380(name)
        : "CONTEXTREGISTRYUNAVAILABLE";
    const std::uint32_t allocation_size = ((size + 3u) & ~3u) + 4u;
    void* payload = alignment != 0
        ? service_heap_allocate_aligned_locked_005de700(
              heap, allocation_size, alignment, region, flags)
        : service_heap_allocate_locked_005dddb0(heap, allocation_size, flags);
    if (payload != nullptr) {
        const std::uint32_t end = service_heap_usable_end_offset_005dede0(
            heap, payload);
        *reinterpret_cast<const char**>(
            static_cast<std::uint8_t*>(payload) + end - 4) = context;
    }
    return payload;
}
