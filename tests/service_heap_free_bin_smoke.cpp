#include <cassert>
#include <cstdint>

extern std::uint32_t service_heap_round_block_005db450(std::uint32_t);
extern std::uint32_t service_heap_large_bin_005ddb00(std::uint32_t);
extern void service_heap_insert_free_bin_005de3e0(
    void*, void*, std::uint32_t);

int main() {
    assert(service_heap_round_block_005db450(0) == 0x10);
    assert(service_heap_round_block_005db450(5) == 0x10);
    assert(service_heap_round_block_005db450(6) == 0x10);
    assert(service_heap_round_block_005db450(14) == 0x18);

    std::uint8_t heap[0x500]{};
    std::uint8_t first[0x50]{};
    std::uint8_t second[0x50]{};
    const std::uint32_t index = 4;
    auto* sentinel = heap + 0x28 + index * 8;
    *reinterpret_cast<void**>(sentinel + 8) = sentinel;
    *reinterpret_cast<void**>(sentinel + 12) = sentinel;
    service_heap_insert_free_bin_005de3e0(heap, first, 0x20);
    assert(*reinterpret_cast<void**>(sentinel + 8) == first);
    assert(*reinterpret_cast<void**>(sentinel + 12) == first);
    assert(*reinterpret_cast<std::uint32_t*>(heap + 0x430) == (1u << index));
    service_heap_insert_free_bin_005de3e0(heap, second, 0x20);
    assert(*reinterpret_cast<void**>(first + 8) == second);
    assert(*reinterpret_cast<void**>(second + 12) == first);

    const std::uint32_t large_index = service_heap_large_bin_005ddb00(0x200);
    sentinel = heap + 0x28 + large_index * 8;
    *reinterpret_cast<void**>(sentinel + 8) = sentinel;
    *reinterpret_cast<void**>(sentinel + 12) = sentinel;
    *reinterpret_cast<std::uint32_t*>(first + 4) = 0x201;
    service_heap_insert_free_bin_005de3e0(heap, first, 0x200);
    assert(*reinterpret_cast<void**>(sentinel + 8) == first);
    assert((*reinterpret_cast<std::uint32_t*>(
               heap + 0x430 + (large_index >> 5) * 4) &
            (1u << (large_index & 31u))) != 0);
}
