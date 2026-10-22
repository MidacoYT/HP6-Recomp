#include <cassert>
#include <cstdint>

extern void* service_heap_allocate_aligned_core_005de780(
    void*, std::uint32_t, std::uint32_t, std::uint32_t,
    std::uint32_t);

namespace {
alignas(64) std::uint8_t block[0x300]{};
std::uint8_t heap[0x600]{};
std::uint32_t requested, passed_flags;
int calls, frees, splits;
}

void* service_heap_allocate_core_005dde30(
    void* arg_heap, std::uint32_t size, std::uint32_t flags) {
    assert(arg_heap == heap);
    ++calls;
    requested = size;
    passed_flags = flags;
    return block + 8;
}
void service_heap_split_prefix_005de4d0(
    void* arg_heap, void* arg_block, std::uint32_t total,
    std::uint32_t consumed) {
    assert(arg_heap == heap && arg_block == block && total == 0x200);
    assert(consumed < total);
    *reinterpret_cast<std::uint32_t*>(block + 4) = consumed | 1u;
    ++splits;
}
void* service_heap_split_tail_005de470(
    void* arg_heap, void* arg_block, std::uint32_t total,
    std::uint32_t wanted) {
    assert(arg_heap == heap && arg_block == block);
    assert(total > wanted && wanted == 0x28);
    ++splits;
    return block + total - wanted;
}
void service_heap_initialize_free_block_005ddc00(
    void*, void*, std::uint32_t) {
    assert(false);
}
void service_heap_free_payload_005de580(void* arg_heap, void*) {
    assert(arg_heap == heap);
    ++frees;
}

int main() {
    assert(service_heap_allocate_aligned_core_005de780(
               heap, 0x20, 8, 0, 0) == block + 8);
    assert(calls == 1 && requested == 0x20 && frees == 0);

    *reinterpret_cast<std::uint32_t*>(block + 4) = 0x201;
    *reinterpret_cast<std::uint32_t*>(block + 0x204) = 9;
    *reinterpret_cast<std::uint32_t*>(block + 0x20c) = 1;
    auto* payload = static_cast<std::uint8_t*>(
        service_heap_allocate_aligned_core_005de780(
            heap, 0x20, 64, 0, 0));
    assert(payload != nullptr);
    assert((reinterpret_cast<std::uintptr_t>(payload) & 63u) == 0);
    assert(requested == 0x78 && passed_flags == 0);
    assert(frees == 2);

    for (std::uint32_t index = 0; index < 0x80; ++index) {
        auto* sentinel = heap + 0x28 + index * 8;
        *reinterpret_cast<void**>(sentinel + 8) = sentinel;
        *reinterpret_cast<void**>(sentinel + 12) = sentinel;
    }
    *reinterpret_cast<void**>(heap + 0x440) = block;
    *reinterpret_cast<std::uint32_t*>(block + 4) = 0x201;
    *reinterpret_cast<std::uint32_t*>(block + 0x204) = 9;
    *reinterpret_cast<std::uint32_t*>(block + 0x20c) = 1;
    payload = static_cast<std::uint8_t*>(
        service_heap_allocate_aligned_core_005de780(
            heap, 0x20, 64, 0, 6));
    assert(payload != nullptr);
    assert((reinterpret_cast<std::uintptr_t>(payload) & 63u) == 0);

    auto* unsorted = heap + 0x30;
    *reinterpret_cast<void**>(unsorted + 8) = block;
    *reinterpret_cast<void**>(unsorted + 12) = block;
    *reinterpret_cast<void**>(block + 8) = unsorted;
    *reinterpret_cast<void**>(block + 12) = unsorted;
    *reinterpret_cast<std::uint32_t*>(block + 4) = 0x201;
    payload = static_cast<std::uint8_t*>(
        service_heap_allocate_aligned_core_005de780(
            heap, 0x20, 64, 0, 7));
    assert(payload == block + 0x1c0);
    assert(splits == 2);
}
