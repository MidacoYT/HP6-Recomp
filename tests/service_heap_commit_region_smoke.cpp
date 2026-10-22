#include <cassert>
#include <cstdint>
#include <windows.h>

extern void* service_heap_commit_region_005df100(
    void*, std::uint32_t);

namespace {
void initialize(std::uint8_t* heap) {
    *reinterpret_cast<std::uint32_t*>(heap + 0x4d4) = 0x1000;
    *reinterpret_cast<std::uint32_t*>(heap + 0x4d8) = 0x10000;
    *reinterpret_cast<std::uint32_t*>(heap + 0x4dc) = 0x1000;
    *reinterpret_cast<void**>(heap + 0x440) = heap + 0x30;
    *reinterpret_cast<void**>(heap + 0x3c) = heap + 0x30;
    *reinterpret_cast<void**>(heap + 0x460) = heap + 0x448;
    *reinterpret_cast<void**>(heap + 0x464) = heap + 0x448;
}
}

int main() {
    std::uint8_t heap[0x500]{};
    initialize(heap);
    auto* block = static_cast<std::uint8_t*>(
        service_heap_commit_region_005df100(heap, 0x100));
    assert(block != nullptr);
    assert(*reinterpret_cast<std::uint32_t*>(block + 4) == 0x101);
    assert(*reinterpret_cast<void**>(heap + 0x440) == block + 0x100);
    assert(*reinterpret_cast<void**>(heap + 0x468) > block + 0x100);
    void* region = *reinterpret_cast<void**>(heap + 0x460);
    assert(VirtualFree(region, 0, MEM_RELEASE) != 0);

    for (auto& byte : heap) byte = 0;
    initialize(heap);
    region = VirtualAlloc(nullptr, 0x10000,
                          MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    assert(region != nullptr);
    *reinterpret_cast<std::uint32_t*>(
        static_cast<std::uint8_t*>(region) + 8) = 0x10000;
    *reinterpret_cast<void**>(
        static_cast<std::uint8_t*>(region) + 0x18) = heap + 0x448;
    *reinterpret_cast<void**>(heap + 0x460) = region;
    *reinterpret_cast<void**>(heap + 0x464) = region;
    block = static_cast<std::uint8_t*>(
        service_heap_commit_region_005df100(heap, 0x100));
    assert(block != nullptr);
    assert(*reinterpret_cast<std::uint32_t*>(
               static_cast<std::uint8_t*>(region) + 4) == 0x1000);
    assert(*reinterpret_cast<std::uint32_t*>(block + 4) == 0x101);
    assert(*reinterpret_cast<void**>(heap + 0x440) == block + 0x100);
    auto* second = static_cast<std::uint8_t*>(
        service_heap_commit_region_005df100(heap, 0x100));
    assert(second == block + 0x100);
    assert(*reinterpret_cast<std::uint32_t*>(
               static_cast<std::uint8_t*>(region) + 4) == 0x2000);
    assert(*reinterpret_cast<std::uint32_t*>(second + 4) == 0x101);
    assert(*reinterpret_cast<void**>(heap + 0x440) == second + 0x100);
    assert(VirtualFree(region, 0, MEM_RELEASE) != 0);
}
