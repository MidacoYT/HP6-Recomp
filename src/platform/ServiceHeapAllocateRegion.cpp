#include <cstdint>
#include <windows.h>

extern void service_heap_lock_005dd290(void* lock);
extern void service_heap_unlock_005dd2a0(void* lock);
extern void* service_heap_reserve_region_005df3a0(
    void* heap, std::uint32_t request, bool commit_all);
extern void service_heap_insert_region_005ddba0(
    void* heap, void* region, bool preserve_old_head);

namespace {
template <typename T>
T& field(void* base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}
} // namespace

// 0x00FF9850..0x00FF9A5F: ECX=heap, EAX=caller region, five stack arguments.
// The low result byte is success. The other result bytes are not meaningful.
std::uint32_t service_heap_allocate_00ff9850(
    void* heap, void* region, std::uint32_t size, std::uint8_t mode,
    std::uint32_t mode2, std::uint32_t ownership, std::uint32_t tag) {
    void* lock = field<void*>(heap, 0x4e4);
    if (lock != nullptr) service_heap_lock_005dd290(lock);

    bool success = false;
    if (region == nullptr) {
        if (size != 0) {
            const std::uint32_t page = field<std::uint32_t>(heap, 0x4d4);
            const std::uint32_t request = (size + page - 1u) & ~(page - 1u);
            void* first = service_heap_reserve_region_005df3a0(
                heap, request, true);
            if (first != nullptr) {
                service_heap_insert_region_005ddba0(heap, first, true);
                success = true;
            }
        }
    } else if (size >= 0x40) {
        auto* bytes = static_cast<std::uint8_t*>(region);
        const std::uint32_t page = field<std::uint32_t>(heap, 0x4d4);
        if (size % page != 0) size &= ~(page - 1u);
        size &= ~0x0fu;

        auto* first = reinterpret_cast<std::uint8_t*>(
            (reinterpret_cast<std::uintptr_t>(bytes + 0x27)) &
            ~std::uintptr_t(7));
        field<void*>(region, 0) = first;
        field<std::uint8_t>(region, 0x0d) = mode;
        field<std::uint8_t>(region, 0x0e) = mode;
        field<std::uint8_t>(region, 0x0f) =
            static_cast<std::uint8_t>(mode2);
        field<std::uint32_t>(region, 4) = size;
        field<std::uint32_t>(region, 8) = size;
        field<std::uint8_t>(region, 0x0c) = 0;
        field<std::uint32_t>(region, 0x10) = ownership;
        field<std::uint32_t>(region, 0x14) = tag;
        field<void*>(region, 0x1c) = static_cast<std::uint8_t*>(heap) + 0x448;
        void* old_first = field<void*>(heap, 0x460);
        field<void*>(region, 0x18) = old_first;
        field<void*>(heap, 0x460) = region;
        field<void*>(old_first, 0x1c) = region;

        if (ownership == 0) {
            MEMORY_BASIC_INFORMATION info{};
            VirtualQuery(region, &info, sizeof(info));
            if (info.State == MEM_RESERVE) field<std::uint32_t>(region, 4) = 0;
        }

        const std::uint32_t span = static_cast<std::uint32_t>(
            bytes + size - first);
        const std::uint32_t block_size = ((span & 0x7ffffff8u) - 9u) & ~7u;
        field<std::uint32_t>(first, 0) = 0;
        field<std::uint32_t>(first, 4) =
            (span & 0x80000007u) | 1u | block_size;
        field<std::uint32_t>(first, block_size) = block_size;
        field<std::uint32_t>(first, block_size + 4) = 8;
        field<std::uint32_t>(first, block_size + 8) = 8;
        field<std::uint32_t>(first, block_size + 12) = 9;
        const bool preserve_old_head = field<void*>(heap, 0x440) !=
            static_cast<std::uint8_t*>(heap) + 0x30;
        service_heap_insert_region_005ddba0(
            heap, first, preserve_old_head);
        success = true;
    }

    if (lock != nullptr) service_heap_unlock_005dd2a0(lock);
    return success ? 1u : 0u;
}
