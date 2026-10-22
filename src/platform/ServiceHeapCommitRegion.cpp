#include <cstdint>
#include <windows.h>

extern void* service_heap_reserve_region_005df3a0(
    void* heap, std::uint32_t request, bool commit_all);
extern void service_heap_insert_region_005ddba0(
    void* heap, void* region, bool preserve_old_head);

namespace {
template <typename T>
T& field(void* base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}
void unlink(void* block) {
    void* next = field<void*>(block, 8);
    void* previous = field<void*>(block, 0x0c);
    field<void*>(next, 0x0c) = previous;
    field<void*>(previous, 8) = next;
}
void append_free(void* heap, void* block) {
    auto* sentinel = static_cast<std::uint8_t*>(heap) + 0x30;
    void* previous = field<void*>(sentinel, 0x0c);
    field<void*>(block, 8) = sentinel;
    field<void*>(block, 0x0c) = previous;
    field<void*>(sentinel, 0x0c) = block;
    field<void*>(previous, 8) = block;
}
void set_end(void* heap) {
    void* head = field<void*>(heap, 0x440);
    const std::uint32_t header = field<std::uint32_t>(head, 4);
    field<void*>(heap, 0x468) = static_cast<std::uint8_t*>(head) +
        ((header >> 1) & 0x3ffffffcu);
}
} // namespace

// 0x005DF100 -> 0x00FF9A60..0x00FF9CF8. EAX=heap, one stack request.
// Returns the free block formed by committing a reserved range, or null.
void* service_heap_commit_region_005df100(
    void* heap, std::uint32_t request) {
    auto* bytes = static_cast<std::uint8_t*>(heap);
    auto* region = static_cast<std::uint8_t*>(field<void*>(heap, 0x460));
    const auto* sentinel = bytes + 0x448;
    std::uint32_t commit_size = 0;
    for (; region != sentinel;
         region = static_cast<std::uint8_t*>(field<void*>(region, 0x18))) {
        const std::uint32_t committed = field<std::uint32_t>(region, 4);
        const std::uint32_t reserved = field<std::uint32_t>(region, 8);
        if (request <= reserved - committed) {
            commit_size = request;
            const std::uint32_t minimum = field<std::uint32_t>(heap, 0x4dc);
            if (commit_size < minimum) commit_size = minimum;
            if (commit_size > reserved - committed)
                commit_size = reserved - committed;
            if (VirtualAlloc(region + committed, commit_size,
                             MEM_COMMIT, PAGE_READWRITE) == nullptr)
                field<std::uint32_t>(region, 8) = committed;
            break;
        }
    }

    std::uint8_t* block = nullptr;
    bool top = false;
    std::uint32_t block_size = 0;
    if (region != sentinel) {
        const std::uint32_t committed = field<std::uint32_t>(region, 4);
        if (committed == 0) {
            block = reinterpret_cast<std::uint8_t*>(
                (reinterpret_cast<std::uintptr_t>(region + 0x27)) &
                ~std::uintptr_t(7));
            field<void*>(region, 0) = block;
            const std::uint32_t span = static_cast<std::uint32_t>(
                region + commit_size - block);
            block_size = ((span & 0x7ffffff8u) - 9u) & ~7u;
            field<std::uint32_t>(block, 0) = 0;
            field<std::uint32_t>(block, 4) =
                (span & 0x80000007u) | 1u | block_size;
            field<std::uint32_t>(block, block_size) = block_size;
            field<std::uint32_t>(block, block_size + 4) = 8;
            field<std::uint32_t>(block, block_size + 8) = 8;
            field<std::uint32_t>(block, block_size + 12) = 9;
            service_heap_insert_region_005ddba0(heap, block, true);
            top = true;
        } else {
            auto* last = region + committed - 0x10;
            if ((field<std::uint8_t>(last, 4) & 1u) != 0) {
                block = last;
                block_size = commit_size;
                field<std::uint32_t>(block, 4) = block_size | 1u;
            } else {
                block = last - field<std::uint32_t>(last, 0);
                unlink(block);
                block_size = field<std::uint32_t>(last, 0) + commit_size;
                field<std::uint32_t>(block, 4) = block_size | 1u;
                top = field<void*>(heap, 0x440) == block;
            }
            auto* tail = block + block_size;
            field<std::uint32_t>(tail, 0) = block_size;
            field<std::uint32_t>(tail, 4) = 8;
            field<std::uint32_t>(tail + 8, 0) = 8;
            field<std::uint32_t>(tail + 8, 4) = 9;
        }
        field<std::uint32_t>(region, 4) += commit_size;
    } else {
        block = static_cast<std::uint8_t*>(
            service_heap_reserve_region_005df3a0(heap, request, false));
        if (block == nullptr) return nullptr;
        service_heap_insert_region_005ddba0(heap, block, true);
        top = true;
        block_size = field<std::uint32_t>(block, 4) & 0x7ffffff8u;
    }

    if (block_size > request) {
        field<std::uint32_t>(block, 4) = request | 1u;
        auto* remainder = block + request;
        const std::uint32_t remainder_size = block_size - request;
        field<std::uint32_t>(remainder, 0) = request;
        field<std::uint32_t>(remainder, 4) = remainder_size;
        field<std::uint32_t>(remainder, remainder_size) = remainder_size;
        if (top) {
            field<void*>(heap, 0x440) = remainder;
            field<void*>(remainder, 0x0c) = remainder;
            field<void*>(remainder, 8) = remainder;
            if (field<std::uint8_t>(heap, 0x46c) == 0) set_end(heap);
        } else {
            append_free(heap, remainder);
        }
    }
    return block;
}
