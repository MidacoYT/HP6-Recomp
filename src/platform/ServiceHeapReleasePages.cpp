// 0x00FF9D00, reached through the 0x005DF570 thunk. Scans tracked
// virtual allocations and decommits trailing pages from large free blocks.
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

extern void service_heap_lock_005dd290(void* lock);
extern void service_heap_unlock_005dd2a0(void* lock);
extern void service_heap_write_tail_005ddc40(void* block);

namespace {
std::uint32_t& word(std::uint8_t* address) {
    return *reinterpret_cast<std::uint32_t*>(address);
}
std::uint8_t*& pointer(std::uint8_t* address) {
    return *reinterpret_cast<std::uint8_t**>(address);
}
} // namespace

void service_heap_release_pages_00ff9d00(void* heap_ptr, std::uint32_t threshold) {
    auto* heap = static_cast<std::uint8_t*>(heap_ptr);
    if (auto* lock = pointer(heap + 0x4e4); lock != nullptr) {
        service_heap_lock_005dd290(lock);
    }

    auto* node = pointer(heap + 0x460);
    while (node != heap + 0x448) {
        if (node[0x0d] != 0) {
            auto* footer = node + word(node + 4) - 0x10;
            if ((*(footer + 4) & 1u) == 0 && node[0x0f] != 0) {
                auto* block = footer - word(footer);
                const auto block_size = word(block + 4) & 0x7ffffff8u;
                if (threshold + 0x40u < block_size) {
                    const auto step = ((word(heap + 0x4dc) >> 1) + 0xffffu) & 0xffff0000u;
                    const auto excess = block_size - (threshold + 0x40u);
                    std::uint32_t decommit_size = step;
                    if (step * 2u < excess) {
                        auto covered = step * 2u;
                        do {
                            covered += step;
                            decommit_size += step;
                        } while (covered < excess);
                    }
                    if (decommit_size < excess) {
#ifdef _WIN32
                        const bool released = VirtualFree(
                            node + word(node + 4) - decommit_size,
                            decommit_size, MEM_DECOMMIT) != 0;
#else
                        const bool released = false;
#endif
                        if (released) {
                            word(node + 4) -= decommit_size;
                            const bool is_top = block == pointer(heap + 0x440);
                            if (!is_top) {
                                auto* previous = pointer(block + 8);
                                auto* next = pointer(block + 0x0c);
                                pointer(previous + 0x0c) = next;
                                pointer(next + 8) = previous;
                            }
                            word(block + 4) = (block_size - decommit_size + 0x10u) |
                                (word(block + 4) & 0x80000007u);
                            service_heap_write_tail_005ddc40(block);
                            if (!is_top) {
                                auto* tail = pointer(heap + 0x3c);
                                pointer(block + 8) = heap + 0x30;
                                pointer(block + 0x0c) = tail;
                                pointer(heap + 0x3c) = block;
                                pointer(tail + 8) = block;
                            }
                        }
                    }
                }
            }
        }
        node = pointer(node + 0x18);
    }

    if (auto* lock = pointer(heap + 0x4e4); lock != nullptr) {
        service_heap_unlock_005dd2a0(lock);
    }
}
