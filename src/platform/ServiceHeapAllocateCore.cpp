#include <cstdint>

extern std::uint32_t service_heap_round_block_005db450(std::uint32_t);
extern std::uint32_t service_heap_large_bin_005ddb00(std::uint32_t);
extern void service_heap_prepare_release_005dec90(void*);
extern void service_heap_insert_free_bin_005de3e0(
    void*, void*, std::uint32_t);
extern void service_heap_split_prefix_005de4d0(
    void*, void*, std::uint32_t, std::uint32_t);
extern void* service_heap_allocate_direct_005df6d0(
    void*, std::uint32_t);
extern void* service_heap_commit_region_005df100(
    void*, std::uint32_t);
extern void service_heap_lock_005dd290(void*);
extern void service_heap_unlock_005dd2a0(void*);

namespace {
template <typename T>
T& at(void* base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}
std::uint32_t size_of(void* block) {
    return at<std::uint32_t>(block, 4) & 0x7ffffff8u;
}
void* next(void* block) { return at<void*>(block, 8); }
void unlink(void* block) {
    void* following = next(block);
    void* previous = at<void*>(block, 0x0c);
    at<void*>(following, 0x0c) = previous;
    at<void*>(previous, 8) = following;
}
bool allowed(void* heap, void* block, std::uint32_t size, bool high) {
    const auto boundary = reinterpret_cast<std::uintptr_t>(
        at<void*>(heap, 0x468));
    const auto address = reinterpret_cast<std::uintptr_t>(block);
    return high ? address + size >= boundary : address < boundary;
}
void* finish_exact(void* block, std::uint32_t size) {
    at<std::uint32_t>(static_cast<std::uint8_t*>(block) + size, 4) |= 1u;
    return static_cast<std::uint8_t*>(block) + 8;
}
void* take_split(void* heap, void* block, std::uint32_t size,
                 std::uint32_t request) {
    service_heap_split_prefix_005de4d0(heap, block, size, request);
    return static_cast<std::uint8_t*>(block) + 8;
}
void set_top_end(void* heap) {
    void* head = at<void*>(heap, 0x440);
    const std::uint32_t header = at<std::uint32_t>(head, 4);
    at<void*>(heap, 0x468) = static_cast<std::uint8_t*>(head) +
        ((header >> 1) & 0x3ffffffcu);
}
using GrowthCallback = bool (*)(void*, std::uint32_t,
                                std::uint32_t, void*);
} // namespace

// 0x005DDE30..0x005DE3DC: EAX=heap, stack=(requested size, flags).
void* service_heap_allocate_core_005dde30(
    void* heap, std::uint32_t requested, std::uint32_t flags) {
    if ((flags & 8u) != 0)
        return service_heap_allocate_direct_005df6d0(heap, requested);
    const std::uint32_t size = service_heap_round_block_005db450(requested);
    std::uint32_t callback_attempts = 0;

    for (;;) {
        const bool high = (flags & 1u) != 0;
        const bool skip_bins = (flags & 0x200u) != 0;
        std::uint32_t index = size < 0x200u
            ? size >> 3 : service_heap_large_bin_005ddb00(size);

        if (!skip_bins) {
            const std::uint32_t small_capacity = at<std::uint32_t>(heap, 4);
            if (!high && size <= small_capacity) {
                void*& head = at<void*>(heap, (size >> 3) * 4u);
                if (head != nullptr) {
                    void* block = head;
                    head = at<void*>(block, 0x0c);
                    at<std::uint32_t>(block, 4) &= 0x7ffffffbu;
                    return static_cast<std::uint8_t*>(block) + 8;
                }
            }

            if (size < 0x200u) {
                auto* sentinel = static_cast<std::uint8_t*>(heap) +
                    0x28 + index * 8u;
                void* block = next(sentinel);
                if (block != sentinel && allowed(heap, block, size, high)) {
                    unlink(block);
                    return finish_exact(block, size);
                }
            } else if ((small_capacity & 1u) != 0) {
                service_heap_prepare_release_005dec90(heap);
            }

            auto* unsorted = static_cast<std::uint8_t*>(heap) + 0x30;
            for (void* block = next(unsorted); block != unsorted;) {
                void* following = next(block);
                const std::uint32_t block_size = size_of(block);
                const bool position = allowed(heap, block, block_size, high);
                if (size < 0x200u && block == at<void*>(heap, 0x444) &&
                    following == unsorted && block_size > size + 0x10u &&
                    position)
                    return take_split(heap, block, block_size, size);
                unlink(block);
                if (block_size == size && position)
                    return finish_exact(block, size);
                service_heap_insert_free_bin_005de3e0(
                    heap, block, block_size);
                block = next(unsorted);
            }

            if (size >= 0x200u) {
                auto* sentinel = static_cast<std::uint8_t*>(heap) +
                    0x28 + index * 8u;
                for (void* block = next(sentinel); block != sentinel;
                     block = next(block)) {
                    const std::uint32_t block_size = size_of(block);
                    if (block_size >= size &&
                        allowed(heap, block, block_size, high))
                        return take_split(heap, block, block_size, size);
                }
            }

            for (std::uint32_t bin = index + 1; bin < 0x80; ++bin) {
                std::uint32_t& bits = at<std::uint32_t>(
                    heap, 0x430 + (bin >> 5) * 4);
                const std::uint32_t mask = 1u << (bin & 31u);
                if ((bits & mask) == 0) continue;
                auto* sentinel = static_cast<std::uint8_t*>(heap) +
                    0x28 + bin * 8u;
                if (next(sentinel) == sentinel) {
                    bits &= ~mask;
                    continue;
                }
                for (void* block = next(sentinel); block != sentinel;
                     block = next(block)) {
                    const std::uint32_t block_size = size_of(block);
                    if (allowed(heap, block, block_size, high))
                        return take_split(heap, block, block_size, size);
                }
            }
        }

        auto* top = static_cast<std::uint8_t*>(at<void*>(heap, 0x440));
        const std::uint32_t top_size = size_of(top);
        if (top_size >= size + 0x10u) {
            std::uint32_t remainder_size;
            std::uint8_t* result;
            std::uint8_t* remainder;
            if (!high) {
                result = top;
                remainder = top + size;
                remainder_size = top_size - size;
                at<std::uint32_t>(result, 4) = size | 1u;
                at<std::uint32_t>(remainder, 4) = remainder_size | 1u;
            } else {
                remainder = top;
                remainder_size = top_size - size;
                result = top + remainder_size;
                at<std::uint32_t>(result, 4) = size;
                at<std::uint32_t>(result + size, 4) |= 1u;
                at<std::uint32_t>(result + size, 0) = size;
            }
            at<void*>(heap, 0x440) = remainder;
            at<void*>(remainder, 8) = remainder;
            at<void*>(remainder, 0x0c) = remainder;
            at<std::uint32_t>(remainder, 4) = remainder_size | 1u;
            at<std::uint32_t>(remainder, remainder_size) = remainder_size;
            set_top_end(heap);
            return result + 8;
        }

        if ((at<std::uint8_t>(heap, 4) & 1u) != 0) {
            service_heap_prepare_release_005dec90(heap);
            continue;
        }
        if (skip_bins) {
            flags &= ~0x200u;
            continue;
        }
        if (at<std::uint8_t>(heap, 0x46d) != 0 &&
            size >= at<std::uint32_t>(heap, 0x494) &&
            static_cast<std::int32_t>(at<std::uint32_t>(heap, 0x488)) <
                static_cast<std::int32_t>(at<std::uint32_t>(heap, 0x490))) {
            if (void* direct = service_heap_allocate_direct_005df6d0(
                    heap, requested))
                return direct;
        }
        if ((flags & 0x40000000u) == 0) {
            flags = high ? ((flags & ~3u) | 0x40000000u)
                         : (flags | 0x40000001u);
            callback_attempts = 0;
            continue;
        }
        if (at<std::uint8_t>(heap, 0x46d) != 0) {
            if (void* block = service_heap_commit_region_005df100(heap, size)) {
                const std::uint32_t block_size = size_of(block);
                at<std::uint32_t>(
                    static_cast<std::uint8_t*>(block) + block_size, 4) |= 1u;
                return static_cast<std::uint8_t*>(block) + 8;
            }
        }
        auto callback = at<GrowthCallback>(heap, 0x4ac);
        if (callback == nullptr || ++callback_attempts >=
            at<std::uint32_t>(heap, 0x4b4))
            return nullptr;
        void* lock = at<void*>(heap, 0x4e4);
        const auto depth = at<std::uint32_t>(lock, 0x18);
        for (std::uint32_t i = 0; i < depth; ++i)
            service_heap_unlock_005dd2a0(lock);
        const bool grew = callback(heap, requested, requested + 0x40u,
                                   at<void*>(heap, 0x4b0));
        for (std::uint32_t i = 0; i < depth; ++i)
            service_heap_lock_005dd290(lock);
        if (!grew) return nullptr;
    }
}
