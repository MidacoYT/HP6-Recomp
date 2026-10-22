// Verified nonzero-header path of 0x005DEC90 in the unprotected HP6 x86
// PE. Walks pending free-block chains and joins adjacent free blocks.
#include <cstdint>

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

void service_heap_prepare_release_after_flag_005dec90(void* heap_ptr) {
    auto* heap = static_cast<std::uint8_t*>(heap_ptr);
    const auto bucket_bytes = word(heap + 4);
    // The calling branch in 0x005DD6B0 proves bucket_bytes bit 0 is set.
    auto* final_slot = heap + (bucket_bytes >> 3) * 4;
    for (auto* slot = heap + 8;; slot += 4) {
        auto* block = pointer(slot);
        if (block != nullptr) {
            pointer(slot) = nullptr;
            do {
                word(block + 4) &= 0x7ffffffbu;
                auto* pending_next = pointer(block + 0x0c);
                std::uint32_t size = word(block + 4) & 0x7ffffff8u;
                auto* next_block = block + size;
                const auto next_size = word(next_block + 4) & 0x7ffffff8u;

                if ((word(block + 4) & 1u) == 0) {
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

                if (block == pointer(heap + 0x440) ||
                    next_block == pointer(heap + 0x440)) {
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
                block = pending_next;
            } while (block != nullptr);
        }
        if (slot == final_slot) break;
    }
    word(heap + 4) &= ~1u;
}
