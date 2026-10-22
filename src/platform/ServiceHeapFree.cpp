// 0x005DE580 jumps to 0x00FF9020 in the unprotected HP6 x86 PE.
// The routine returns blocks to a small-bin chain, merges adjacent free
// blocks, or releases a virtual allocation according to its header flags.
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

extern bool service_heap_should_release_005ddb50(void* end, void* heap);
extern void service_heap_release_pages_00ff9d00(void* heap, std::uint32_t threshold);

namespace {
std::uint32_t& word(std::uint8_t* address) {
    return *reinterpret_cast<std::uint32_t*>(address);
}

std::uint8_t*& pointer(std::uint8_t* address) {
    return *reinterpret_cast<std::uint8_t**>(address);
}

void unlink_free_block(std::uint8_t* block) {
    auto* previous = pointer(block + 8);
    auto* next = pointer(block + 0x0c);
    pointer(previous + 0x0c) = next;
    pointer(next + 8) = previous;
}
} // namespace

void service_heap_free_payload_005de580(void* heap_ptr, void* payload_ptr) {
    if (payload_ptr == nullptr) return;
    auto* heap = static_cast<std::uint8_t*>(heap_ptr);
    auto* payload = static_cast<std::uint8_t*>(payload_ptr);
    auto* block = payload - 8;
    const std::uint32_t flags = word(payload - 4);
    std::uint32_t size = flags & 0x7ffffff8u;

    if (size <= word(heap + 4) &&
        reinterpret_cast<std::uintptr_t>(block) <
            reinterpret_cast<std::uintptr_t>(pointer(heap + 0x468))) {
        word(heap + 4) |= 1u;
        auto* bucket = heap + (size >> 3) * 4;
        word(payload - 4) |= 0x80000004u;
        pointer(payload + 4) = pointer(bucket);
        pointer(bucket) = block;
        return;
    }

    if ((flags & 2u) == 0) {
        auto* next_block = block + size;
        const auto next_size = word(next_block + 4) & 0x7ffffff8u;
        if ((flags & 1u) == 0) {
            const auto previous_size = word(block);
            block -= previous_size;
            size += previous_size;
            word(block + 4) = size | 1u;
            word(next_block) = size;
            unlink_free_block(block);
        }
        if ((*(next_block + next_size + 4) & 1u) == 0) {
            unlink_free_block(next_block);
            size += next_size;
            word(block + 4) = size | 1u;
            word(block + size) = size;
        } else {
            word(next_block + 4) &= ~1u;
            word(next_block) = size;
        }

        if (block == pointer(heap + 0x440) || next_block == pointer(heap + 0x440)) {
            pointer(heap + 0x440) = block;
            pointer(block + 0x0c) = block;
            pointer(block + 8) = block;
            word(block + 4) = size | 1u;
            word(block + size) = size;
            pointer(heap + 0x468) = block +
                ((word(block + 4) >> 1) & 0x3ffffffcu);
        } else {
            auto* tail = pointer(heap + 0x3c);
            pointer(block + 8) = heap + 0x30;
            pointer(block + 0x0c) = tail;
            pointer(heap + 0x3c) = block;
            pointer(tail + 8) = block;
        }

        if (size > 0xffffu &&
            service_heap_should_release_005ddb50(block + size, heap) &&
            word(heap + 0x4c8) <= size) {
            service_heap_release_pages_00ff9d00(heap, word(heap + 0x4cc));
        }
    } else {
        const auto previous_size = word(block);
        auto* previous = pointer(payload + size);
        auto* next = pointer(payload + size + 4);
        pointer(previous + 0x0c) = next;
        pointer(next + 8) = previous;
        word(heap + 0x48c) -= previous_size + 0x10u + size;
        --word(heap + 0x488);
#ifdef _WIN32
        VirtualFree(block - previous_size, 0, MEM_RELEASE);
#endif
    }
}
