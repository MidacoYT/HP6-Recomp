#include <cstdint>

namespace {
template <typename T>
T& field(void* base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}
} // namespace

// 0x005DDBA0..0x005DDBFE: EAX=heap, EDX=new region, stack=preserve old head.
void service_heap_insert_region_005ddba0(
    void* heap, void* region, bool preserve_old_head) {
    void* old_head = field<void*>(heap, 0x440);
    field<void*>(heap, 0x440) = region;
    field<void*>(region, 0x0c) = region;
    field<void*>(region, 0x08) = region;
    auto* sentinel = static_cast<std::uint8_t*>(heap) + 0x30;
    if (old_head != sentinel && preserve_old_head) {
        void* tail = field<void*>(sentinel, 0x0c);
        field<void*>(old_head, 0x08) = sentinel;
        field<void*>(old_head, 0x0c) = tail;
        field<void*>(sentinel, 0x0c) = old_head;
        field<void*>(tail, 0x08) = old_head;
    }
    if (field<std::uint8_t>(heap, 0x46c) == 0) {
        const std::uint32_t header = field<std::uint32_t>(region, 4);
        const std::uint32_t offset = (header >> 1) & 0x3ffffffcu;
        field<void*>(heap, 0x468) = static_cast<std::uint8_t*>(region) + offset;
    }
}
