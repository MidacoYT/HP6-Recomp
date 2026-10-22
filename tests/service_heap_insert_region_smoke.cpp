#include <cassert>
#include <cstdint>

extern void service_heap_insert_region_005ddba0(void*, void*, bool);

int main() {
    std::uint8_t heap[0x500]{};
    std::uint8_t region[0x100]{};
    auto* sentinel = heap + 0x30;
    *reinterpret_cast<void**>(heap + 0x440) = sentinel;
    *reinterpret_cast<void**>(sentinel + 8) = sentinel;
    *reinterpret_cast<void**>(sentinel + 12) = sentinel;
    *reinterpret_cast<std::uint32_t*>(region + 4) = 0x82;

    service_heap_insert_region_005ddba0(heap, region, true);
    assert(*reinterpret_cast<void**>(heap + 0x440) == region);
    assert(*reinterpret_cast<void**>(region + 8) == region);
    assert(*reinterpret_cast<void**>(region + 12) == region);
    assert(*reinterpret_cast<void**>(heap + 0x468) == region + 0x40);
    assert(*reinterpret_cast<void**>(sentinel + 12) == sentinel);

    std::uint8_t second[0x100]{};
    *reinterpret_cast<std::uint32_t*>(second + 4) = 0x42;
    service_heap_insert_region_005ddba0(heap, second, true);
    assert(*reinterpret_cast<void**>(heap + 0x440) == second);
    assert(*reinterpret_cast<void**>(sentinel + 12) == region);
    assert(*reinterpret_cast<void**>(region + 8) == sentinel);
    assert(*reinterpret_cast<void**>(region + 12) == sentinel);
    assert(*reinterpret_cast<void**>(heap + 0x468) == second + 0x20);

    heap[0x46c] = 1;
    service_heap_insert_region_005ddba0(heap, region, false);
    assert(*reinterpret_cast<void**>(heap + 0x468) == second + 0x20);
    assert(*reinterpret_cast<void**>(sentinel + 12) == region);
}
