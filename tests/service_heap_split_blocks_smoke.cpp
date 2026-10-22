#include <cassert>
#include <cstdint>

extern void* service_heap_split_tail_005de470(
    void*, void*, std::uint32_t, std::uint32_t);
extern void service_heap_split_prefix_005de4d0(
    void*, void*, std::uint32_t, std::uint32_t);
extern void service_heap_initialize_free_block_005ddc00(
    void*, void*, std::uint32_t);

int main() {
    std::uint8_t heap[0x500]{};
    std::uint8_t storage[0x200]{};
    auto* sentinel = heap + 0x30;
    *reinterpret_cast<void**>(sentinel + 8) = sentinel;
    *reinterpret_cast<void**>(sentinel + 12) = sentinel;
    *reinterpret_cast<void**>(heap + 0x440) = storage;
    *reinterpret_cast<std::uint32_t*>(storage + 4) = 0x101;

    service_heap_initialize_free_block_005ddc00(heap, storage, 0x100);
    assert(*reinterpret_cast<void**>(storage + 8) == storage);
    assert(*reinterpret_cast<std::uint32_t*>(storage + 0x100) == 0x100);
    assert(*reinterpret_cast<void**>(heap + 0x468) == storage + 0x80);

    auto* tail = static_cast<std::uint8_t*>(
        service_heap_split_tail_005de470(heap, storage, 0x100, 0x40));
    assert(tail == storage + 0xc0);
    assert(*reinterpret_cast<void**>(sentinel + 12) == storage);
    assert(*reinterpret_cast<std::uint32_t*>(storage + 4) == 0xc1);
    assert(*reinterpret_cast<std::uint32_t*>(tail) == 0xc0);
    assert(*reinterpret_cast<void**>(heap + 0x444) == storage);

    service_heap_split_prefix_005de4d0(heap, storage, 0xc0, 0x40);
    assert(*reinterpret_cast<void**>(sentinel + 12) == storage + 0x40);
    assert(*reinterpret_cast<std::uint32_t*>(storage + 4) == 0x41);
    assert(*reinterpret_cast<std::uint32_t*>(storage + 0x44) == 0x81);
    assert(*reinterpret_cast<void**>(heap + 0x444) == storage + 0x40);

    std::uint8_t small[0x40]{};
    assert(service_heap_split_tail_005de470(
               heap, small, 0x20, 0x18) == small);
    assert(*reinterpret_cast<std::uint32_t*>(small + 0x24) == 1);
}
