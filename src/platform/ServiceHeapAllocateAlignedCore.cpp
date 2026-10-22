#include <cstdint>

extern void* service_heap_allocate_core_005dde30(
    void*, std::uint32_t, std::uint32_t);
extern std::uint32_t service_heap_round_block_005db450(std::uint32_t);
extern std::uint32_t service_heap_large_bin_005ddb00(std::uint32_t);
extern std::uint32_t service_heap_aligned_split_address_005db4d0(
    std::uint32_t, std::uint32_t, void*, std::uint32_t);
extern void service_heap_split_prefix_005de4d0(
    void*, void*, std::uint32_t, std::uint32_t);
extern void* service_heap_split_tail_005de470(
    void*, void*, std::uint32_t, std::uint32_t);
extern void service_heap_initialize_free_block_005ddc00(
    void*, void*, std::uint32_t);
extern void service_heap_free_payload_005de580(void*, void*);

namespace {
template <typename T>
T& at(void* base, std::uint32_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}
std::uint32_t block_size(void* block) {
    return at<std::uint32_t>(block, 4) & 0x7ffffff8u;
}
std::uint32_t next_power_of_two(std::uint32_t value) {
    if ((value & (value - 1u)) == 0) return value;
    --value;
    value |= value >> 1;
    value |= value >> 2;
    value |= value >> 4;
    value |= value >> 8;
    value |= value >> 16;
    return value + 1;
}
std::uint8_t* aligned_block(void* block, std::uint32_t alignment,
                            std::uint32_t offset) {
    const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(block);
    return reinterpret_cast<std::uint8_t*>(
        ((address + alignment + 7u + offset) &
         ~(std::uintptr_t(alignment) - 1u)) - offset - 8u);
}
void* take_free_aligned(void* heap, void* block,
                        std::uint32_t total, std::uint32_t request,
                        std::uint8_t* chosen) {
    const std::uint32_t consumed = static_cast<std::uint32_t>(
        chosen - static_cast<std::uint8_t*>(block)) + request;
    service_heap_split_prefix_005de4d0(heap, block, total, consumed);
    service_heap_split_tail_005de470(
        heap, block, block_size(block), request);
    return chosen + 8;
}
} // namespace

// 0x005DE780 -> 0x00FF9220..0x00FF96F7. EAX=request, ECX=heap;
// stack arguments are alignment, offset, and flags after the heap pointer.
void* service_heap_allocate_aligned_core_005de780(
    void* heap, std::uint32_t request, std::uint32_t alignment,
    std::uint32_t offset, std::uint32_t flags) {
    if (alignment < 9 && offset == 0 && (flags & 0x400u) == 0)
        return service_heap_allocate_core_005dde30(heap, request, flags);
    if (alignment < 0x10u) alignment = 0x10u;
    alignment = next_power_of_two(alignment);
    if ((offset & 7u) != 0) offset = (offset + 7u) & ~7u;
    const std::uint32_t needed = service_heap_round_block_005db450(request);

    if ((flags & 6u) == 6u) {
        auto* bytes = static_cast<std::uint8_t*>(heap);
        const std::uint32_t first_bin = needed < 0x200u
            ? needed >> 3 : service_heap_large_bin_005ddb00(needed);
        auto* top = static_cast<std::uint8_t*>(at<void*>(heap, 0x440));
        std::uint8_t* low = nullptr;
        std::uint8_t* high = nullptr;
        for (std::uint32_t iteration = 0;
             first_bin + iteration <= 0x80u; ++iteration) {
            auto* sentinel = iteration == 0
                ? bytes + 0x30
                : bytes + 0x28 + (first_bin + iteration - 1u) * 8u;
            for (auto* block = static_cast<std::uint8_t*>(
                     at<void*>(sentinel, 0x0c));
                 block != sentinel;
                 block = static_cast<std::uint8_t*>(at<void*>(block, 0x0c))) {
                const std::uint32_t size = block_size(block);
                auto* candidate = aligned_block(block, alignment, offset);
                if (size < needed || candidate == nullptr ||
                    reinterpret_cast<std::uintptr_t>(candidate) + needed >
                        reinterpret_cast<std::uintptr_t>(block) + size)
                    continue;
                const auto address = reinterpret_cast<std::uintptr_t>(block);
                if (address <
                        (reinterpret_cast<std::uintptr_t>(top) - 0x1400000u) &&
                    (low == nullptr || address <
                        reinterpret_cast<std::uintptr_t>(low)))
                    low = block;
                if (address >= reinterpret_cast<std::uintptr_t>(top) &&
                    (high == nullptr || address >
                        reinterpret_cast<std::uintptr_t>(high)))
                    high = block;
            }
        }
        if (high != nullptr && (flags & 1u) != 0) {
            const std::uint32_t size = block_size(high);
            auto* candidate = reinterpret_cast<std::uint8_t*>(
                ((reinterpret_cast<std::uintptr_t>(high + size - needed +
                    offset + 8u) &
                    ~(std::uintptr_t(alignment) - 1u)) - offset - 8u));
            if (candidate + needed <= high + size)
                return take_free_aligned(heap, high, size, needed, candidate);
        }
        const std::uint32_t top_size = block_size(top);
        if (top_size > 0x1400000u) flags &= ~0x100u;
        if (low != nullptr &&
            (((flags & 1u) == 0) || ((flags & 0x100u) != 0))) {
            return take_free_aligned(
                heap, low, block_size(low), needed,
                aligned_block(low, alignment, offset));
        }
        if ((flags & 0x100u) == 0 && (flags & 1u) != 0 &&
            needed + alignment + 0x10u + offset <= top_size) {
            const std::uint32_t address =
                service_heap_aligned_split_address_005db4d0(
                    alignment, offset, top, needed);
            auto* chosen = reinterpret_cast<std::uint8_t*>(
                static_cast<std::uintptr_t>(address));
            const std::uint32_t size = top_size -
                static_cast<std::uint32_t>(chosen - top);
            at<std::uint32_t>(chosen, 4) = size;
            at<std::uint32_t>(chosen + size, 4) |= 1u;
            at<std::uint32_t>(chosen + size, 0) = size;
            at<void*>(heap, 0x440) = top;
            service_heap_initialize_free_block_005ddc00(
                heap, top, static_cast<std::uint32_t>(chosen - top));
            return chosen + 8;
        }
    }

    void* payload = service_heap_allocate_core_005dde30(
        heap, needed + alignment + 0x10u + offset, flags);
    if (payload == nullptr) return nullptr;
    auto* block = static_cast<std::uint8_t*>(payload) - 8;
    auto* chosen = block;
    if (((reinterpret_cast<std::uintptr_t>(payload) + offset) &
         (alignment - 1u)) != 0) {
        chosen = reinterpret_cast<std::uint8_t*>(
            ((reinterpret_cast<std::uintptr_t>(payload) + offset +
              alignment - 1u) & ~(std::uintptr_t(alignment) - 1u)) -
            offset - 8u);
        if (chosen - block < 0x10) chosen += alignment;
        const std::uint32_t prefix = static_cast<std::uint32_t>(chosen - block);
        const std::uint32_t remaining = block_size(block) - prefix;
        if ((at<std::uint32_t>(block, 4) & 2u) != 0) {
            at<std::uint32_t>(chosen, 0) = at<std::uint32_t>(block, 0) + prefix;
            at<std::uint32_t>(chosen, 4) = remaining | 2u;
            at<std::uint32_t>(chosen, remaining) = remaining;
            return chosen + 8;
        }
        at<std::uint32_t>(chosen, 4) = remaining | 1u;
        at<std::uint32_t>(chosen + remaining, 4) |= 1u;
        at<std::uint32_t>(chosen, remaining) = remaining;
        at<std::uint32_t>(block, 4) =
            (at<std::uint32_t>(block, 4) & 0x80000007u) | prefix;
        at<std::uint32_t>(chosen, 0) = prefix;
        const std::uint32_t previous = at<std::uint32_t>(heap, 0x478);
        at<std::uint32_t>(heap, 0x478) = 0;
        service_heap_free_payload_005de580(heap, block + 8);
        at<std::uint32_t>(heap, 0x478) = previous;
    }
    if ((at<std::uint32_t>(chosen, 4) & 2u) == 0) {
        const std::uint32_t size = block_size(chosen);
        if (needed + 0x10u < size) {
            const std::uint32_t remainder_size = size - needed;
            auto* remainder = chosen + needed;
            at<std::uint32_t>(remainder, 4) = remainder_size | 1u;
            at<std::uint32_t>(chosen, 4) =
                (at<std::uint32_t>(chosen, 4) & 0x80000007u) | needed;
            const std::uint32_t next_size =
                block_size(remainder + remainder_size);
            if ((at<std::uint8_t>(remainder + remainder_size + next_size, 4)
                 & 1u) != 0) {
                const std::uint32_t previous = at<std::uint32_t>(heap, 0x478);
                at<std::uint32_t>(heap, 0x478) = 0;
                service_heap_free_payload_005de580(heap, remainder + 8);
                at<std::uint32_t>(heap, 0x478) = previous;
            } else {
                void* after = remainder + remainder_size;
                void* following = at<void*>(after, 8);
                void* prior = at<void*>(after, 0x0c);
                at<void*>(following, 0x0c) = prior;
                at<void*>(prior, 8) = following;
                const std::uint32_t combined = remainder_size + next_size;
                at<std::uint32_t>(remainder, 4) = combined | 1u;
                at<std::uint32_t>(remainder, combined) = combined;
                auto* top = static_cast<std::uint8_t*>(at<void*>(heap, 0x440));
                if (remainder == top || after == top) {
                    at<void*>(heap, 0x440) = remainder;
                    service_heap_initialize_free_block_005ddc00(
                        heap, remainder, combined);
                } else {
                    auto* sentinel = static_cast<std::uint8_t*>(heap) + 0x30;
                    void* tail = at<void*>(sentinel, 0x0c);
                    at<void*>(remainder, 8) = sentinel;
                    at<void*>(remainder, 0x0c) = tail;
                    at<void*>(sentinel, 0x0c) = remainder;
                    at<void*>(tail, 8) = remainder;
                }
            }
        }
    }
    return chosen + 8;
}
