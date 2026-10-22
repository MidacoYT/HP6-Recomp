#include <cassert>
#include <cstdint>
#include <windows.h>

extern void* service_heap_allocate_direct_005df6d0(
    void*, std::uint32_t);

int main() {
    std::uint8_t heap[0x500]{};
    *reinterpret_cast<std::uint32_t*>(heap + 0x4d4) = 0x1000;
    *reinterpret_cast<void**>(heap + 0x4a8) = heap + 0x49c;
    *reinterpret_cast<void**>(heap + 0x49c + 8) = heap + 0x49c;
    auto* payload = static_cast<std::uint8_t*>(
        service_heap_allocate_direct_005df6d0(heap, 0x1000));
    assert(payload != nullptr);
    auto* block = payload - 8;
    assert((*reinterpret_cast<std::uint32_t*>(block + 4) & 2u) != 0);
    assert(*reinterpret_cast<std::uint32_t*>(heap + 0x488) == 1);
    assert(*reinterpret_cast<std::uint32_t*>(heap + 0x48c) == 0x2000);
    auto* tail = static_cast<std::uint8_t*>(
        *reinterpret_cast<void**>(heap + 0x4a8));
    assert(*reinterpret_cast<std::uint32_t*>(tail + 4) == 0x13);
    assert(*reinterpret_cast<void**>(tail + 8) == heap + 0x49c);
    assert(*reinterpret_cast<void**>(tail + 12) == heap + 0x49c);
    assert(VirtualFree(block, 0, MEM_RELEASE) != 0);

    assert(service_heap_allocate_direct_005df6d0(
               heap, 0xfffffff0u) == nullptr);
}
