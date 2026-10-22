#include <cstdint>

extern std::uint32_t service_heap_large_bin_005ddb00(std::uint32_t size);

namespace {
template <typename T>
T& field(void* base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}
} // namespace

// 0x005DB450 -> 0x00FF88D0: minimum allocation block size and 8-byte round.
std::uint32_t service_heap_round_block_005db450(std::uint32_t size) {
    const std::uint32_t adjusted = size + 0x0bu;
    return adjusted > 0x10u ? (adjusted & ~7u) : 0x10u;
}

// 0x005DE3E0..0x005DE46E: insert a free block into its size bin.
void service_heap_insert_free_bin_005de3e0(
    void* heap, void* block, std::uint32_t size) {
    const std::uint32_t index = size < 0x200u
        ? size >> 3 : service_heap_large_bin_005ddb00(size);
    auto* sentinel = static_cast<std::uint8_t*>(heap) + 0x28 + index * 8u;
    void* previous = field<void*>(sentinel, 0x0c);
    void* next = sentinel;
    if (size >= 0x200u && previous != sentinel) {
        void* first = field<void*>(sentinel, 8);
        if (size < field<std::uint32_t>(first, 4)) {
            previous = sentinel;
            next = first;
        } else {
            while (size < (field<std::uint32_t>(previous, 4) &
                           0x7ffffff8u))
                previous = field<void*>(previous, 0x0c);
            next = field<void*>(previous, 8);
        }
    }
    field<std::uint32_t>(heap, 0x430 + (index >> 5) * 4) |=
        1u << (index & 31u);
    field<void*>(block, 8) = next;
    field<void*>(block, 0x0c) = previous;
    field<void*>(previous, 8) = block;
    field<void*>(next, 0x0c) = block;
}
