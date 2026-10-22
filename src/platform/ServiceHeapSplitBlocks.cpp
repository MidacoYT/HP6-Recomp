#include <cstdint>

namespace {
template <typename T>
T& field(void* base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}
void insert_free(void* heap, void* block, std::uint32_t size) {
    auto* sentinel = static_cast<std::uint8_t*>(heap) + 0x30;
    void* tail = field<void*>(sentinel, 0x0c);
    field<void*>(block, 8) = sentinel;
    field<void*>(block, 0x0c) = tail;
    field<void*>(sentinel, 0x0c) = block;
    field<void*>(tail, 8) = block;
    if (size < 0x200) field<void*>(heap, 0x444) = block;
}
} // namespace

// 0x005DE470..0x005DE4C5: leave a free prefix and return a tail block.
void* service_heap_split_tail_005de470(
    void* heap, void* block, std::uint32_t total,
    std::uint32_t tail_size) {
    const std::uint32_t prefix = total - tail_size;
    auto* bytes = static_cast<std::uint8_t*>(block);
    if (prefix < 0x10) {
        field<std::uint32_t>(bytes + total, 4) |= 1u;
        return block;
    }
    insert_free(heap, block, prefix);
    auto* tail = bytes + prefix;
    field<std::uint32_t>(tail, 4) = tail_size;
    field<std::uint32_t>(tail + tail_size, 4) |= 1u;
    field<std::uint32_t>(block, 4) = prefix | 1u;
    field<std::uint32_t>(tail, 0) = prefix;
    return tail;
}

// 0x005DE4D0..0x005DE53C: unlink a free block, optionally keep its tail.
void service_heap_split_prefix_005de4d0(
    void* heap, void* block, std::uint32_t total,
    std::uint32_t prefix) {
    auto* bytes = static_cast<std::uint8_t*>(block);
    void* next = field<void*>(block, 8);
    void* previous = field<void*>(block, 0x0c);
    field<void*>(next, 0x0c) = previous;
    field<void*>(previous, 8) = next;
    const std::uint32_t tail_size = total - prefix;
    if (tail_size < 0x10) {
        field<std::uint32_t>(bytes + total, 4) |= 1u;
        return;
    }
    auto* tail = bytes + prefix;
    insert_free(heap, tail, prefix);
    field<std::uint32_t>(block, 4) = prefix | 1u;
    field<std::uint32_t>(tail, 4) = tail_size | 1u;
    field<std::uint32_t>(tail + tail_size, 0) = tail_size;
}

// 0x005DDC00..0x005DDC32: initialize a standalone free block and end cursor.
void service_heap_initialize_free_block_005ddc00(
    void* heap, void* block, std::uint32_t size) {
    field<void*>(block, 8) = block;
    field<void*>(block, 0x0c) = block;
    field<std::uint32_t>(block, 4) = size | 1u;
    field<std::uint32_t>(block, size) = size;
    void* head = field<void*>(heap, 0x440);
    const std::uint32_t header = field<std::uint32_t>(head, 4);
    field<void*>(heap, 0x468) = static_cast<std::uint8_t*>(head) +
        ((header >> 1) & 0x3ffffffcu);
}
