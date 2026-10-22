#include <cstdint>
#include <windows.h>

namespace {
template <typename T>
T& field(void* base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}
} // namespace

// 0x005DF6D0 -> 0x00FF9E70..0x00FF9F1F. EDI=heap, ECX=request.
// Direct VirtualAlloc allocation for the large-block path of 0x005DDE30.
void* service_heap_allocate_direct_005df6d0(
    void* heap, std::uint32_t request) {
    const std::uint32_t page = field<std::uint32_t>(heap, 0x4d4);
    const std::uint32_t total = (request + page + 0x1eu) & ~(page - 1u);
    if (total < request) return nullptr;
    const DWORD flags = field<std::uint8_t>(heap, 0x498) != 0
        ? (MEM_RESERVE | MEM_COMMIT | MEM_TOP_DOWN)
        : (MEM_RESERVE | MEM_COMMIT);
    auto* allocation = static_cast<std::uint8_t*>(
        VirtualAlloc(nullptr, total, flags, PAGE_READWRITE));
    if (allocation == nullptr) return nullptr;
    field<std::uint32_t>(heap, 0x48c) += total;
    field<std::uint32_t>(heap, 0x488) += 1;
    std::uint32_t block_size = total - 0x10u;
    auto* block = allocation;
    const std::uint32_t skew =
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(block) & 7u);
    if (skew != 0) {
        const std::uint32_t adjustment = 8u - skew;
        block += adjustment;
        block_size -= adjustment;
        field<std::uint32_t>(block, 0) = adjustment;
        field<std::uint32_t>(block, 4) = (block_size - adjustment) | 2u;
    } else {
        field<std::uint32_t>(block, 0) = 0;
        field<std::uint32_t>(block, 4) = block_size | 2u;
    }
    auto* tail = block + block_size;
    field<std::uint32_t>(tail, 0) = block_size;
    field<std::uint32_t>(tail, 4) = 0x13;
    auto* sentinel = static_cast<std::uint8_t*>(heap) + 0x49c;
    void* previous = field<void*>(heap, 0x4a8);
    field<void*>(tail, 8) = sentinel;
    field<void*>(tail, 0x0c) = previous;
    field<void*>(sentinel, 0x0c) = tail;
    field<void*>(previous, 8) = tail;
    return block + 8;
}
