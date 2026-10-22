#include <cstdint>
#include <windows.h>

namespace {
template <typename T>
T& field(void* base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}
std::uint32_t round_up(std::uint32_t value, std::uint32_t alignment) {
    return (value + alignment - 1u) & ~(alignment - 1u);
}
} // namespace

// 0x005DF3A0..0x005DF514. ESI=heap, ECX=request, stack=commit all.
// This function reserves/commits pages and initializes the region header and
// boundary tags; insertion into the heap free lists is done by 0x005DDBA0.
void* service_heap_reserve_region_005df3a0(
    void* heap, std::uint32_t request, bool commit_all) {
    const std::uint32_t page = field<std::uint32_t>(heap, 0x4d4);
    std::uint32_t committed = round_up(request, page);
    std::uint32_t reserve = round_up(
        committed < field<std::uint32_t>(heap, 0x4d8)
            ? field<std::uint32_t>(heap, 0x4d8) : committed, page);
    std::uint32_t reserved = round_up(reserve, 0x10000);
    auto* region = static_cast<std::uint8_t*>(
        VirtualAlloc(nullptr, reserved, MEM_RESERVE, PAGE_READWRITE));
    std::uint32_t flags = 1;
    if (region != nullptr) {
        const std::uint32_t minimum = field<std::uint32_t>(heap, 0x4dc);
        if (committed < minimum) committed = round_up(minimum, page);
        if (committed > reserved || commit_all) committed = reserved;
        region = static_cast<std::uint8_t*>(
            VirtualAlloc(region, committed, MEM_COMMIT, PAGE_READWRITE));
    }
    if (region == nullptr) {
        reserve = field<std::uint32_t>(heap, 0x4d8) > round_up(request, page)
            ? round_up(field<std::uint32_t>(heap, 0x4d8), page)
            : round_up(request, page);
        const DWORD allocation_type = field<std::uint8_t>(heap, 0x498)
            ? (MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN)
            : (MEM_RESERVE | MEM_COMMIT);
        region = static_cast<std::uint8_t*>(
            VirtualAlloc(nullptr, reserve, allocation_type, PAGE_READWRITE));
        if (region == nullptr) return nullptr;
        flags = 3;
        reserved = reserve;
        committed = reserve;
    }

    auto* first = reinterpret_cast<std::uint8_t*>(
        (reinterpret_cast<std::uintptr_t>(region + 0x27)) & ~std::uintptr_t(7));
    field<void*>(region, 0) = first;
    field<std::uint32_t>(region, 4) = committed;
    field<std::uint32_t>(region, 8) = reserved;
    field<std::uint8_t>(region, 0x0c) = static_cast<std::uint8_t>(flags >> 1);
    const bool has_regions = field<void*>(heap, 0x464) !=
                             static_cast<std::uint8_t*>(heap) + 0x448;
    field<std::uint8_t>(region, 0x0d) = has_regions;
    field<std::uint8_t>(region, 0x0e) = 1;
    field<std::uint8_t>(region, 0x0f) = has_regions;
    field<std::uint32_t>(region, 0x10) = 0;
    field<std::uint32_t>(region, 0x14) = 0;
    void* old_first = field<void*>(heap, 0x460);
    field<void*>(region, 0x18) = old_first;
    field<void*>(region, 0x1c) = static_cast<std::uint8_t*>(heap) + 0x448;
    field<void*>(heap, 0x460) = region;
    field<void*>(old_first, 0x1c) = region;
    if (field<void*>(heap, 0x464) != field<void*>(heap, 0x460))
        field<std::uint8_t>(heap, 0x46c) = 1;

    const std::uint32_t span = static_cast<std::uint32_t>(
        region + committed - first);
    const std::uint32_t block_size = ((span & 0x7ffffff8u) - 9u) & ~7u;
    field<std::uint32_t>(first, 0) = 0;
    field<std::uint32_t>(first, 4) =
        (span & 0x80000007u) | flags | block_size;
    field<std::uint32_t>(first, block_size) = block_size;
    field<std::uint32_t>(first, block_size + 4) = 8;
    field<std::uint32_t>(first, block_size + 8) = 8;
    field<std::uint32_t>(first, block_size + 12) = 9;
    return first;
}
