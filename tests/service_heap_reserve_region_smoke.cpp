#include <cassert>
#include <cstdint>
#include <windows.h>

extern void* service_heap_reserve_region_005df3a0(
    void*, std::uint32_t, bool);

int main() {
    std::uint8_t heap[0x500]{};
    *reinterpret_cast<std::uint32_t*>(heap + 0x4d4) = 0x1000;
    *reinterpret_cast<std::uint32_t*>(heap + 0x4d8) = 0x10000;
    *reinterpret_cast<std::uint32_t*>(heap + 0x4dc) = 0x1000;
    *reinterpret_cast<void**>(heap + 0x460) = heap + 0x448;
    *reinterpret_cast<void**>(heap + 0x464) = heap + 0x448;
    void* first = service_heap_reserve_region_005df3a0(heap, 0x2000, false);
    assert(first != nullptr);
    void* region = *reinterpret_cast<void**>(heap + 0x460);
    assert(*reinterpret_cast<void**>(region) == first);
    assert(*reinterpret_cast<std::uint32_t*>(
               static_cast<std::uint8_t*>(region) + 4) == 0x2000);
    assert(*reinterpret_cast<std::uint32_t*>(
               static_cast<std::uint8_t*>(region) + 8) == 0x10000);
    assert(*reinterpret_cast<void**>(heap + 0x464) == region);
    assert(heap[0x46c] == 0);
    assert((*reinterpret_cast<std::uint32_t*>(
               static_cast<std::uint8_t*>(first) + 4) & 1u) != 0);
    assert(VirtualFree(region, 0, MEM_RELEASE) != 0);
}
