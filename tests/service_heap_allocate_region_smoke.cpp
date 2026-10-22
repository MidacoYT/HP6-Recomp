#include <cassert>
#include <cstdint>
#include <windows.h>

extern std::uint32_t service_heap_allocate_00ff9850(
    void*, void*, std::uint32_t, std::uint8_t, std::uint32_t,
    std::uint32_t, std::uint32_t);

namespace {
int locks, unlocks;
std::uint8_t heap[0x500]{};

void reset_heap() {
    for (auto& byte : heap) byte = 0;
    *reinterpret_cast<std::uint32_t*>(heap + 0x4d4) = 0x1000;
    *reinterpret_cast<std::uint32_t*>(heap + 0x4d8) = 0x10000;
    *reinterpret_cast<std::uint32_t*>(heap + 0x4dc) = 0x1000;
    *reinterpret_cast<void**>(heap + 0x440) = heap + 0x30;
    *reinterpret_cast<void**>(heap + 0x3c) = heap + 0x30;
    *reinterpret_cast<void**>(heap + 0x460) = heap + 0x448;
    *reinterpret_cast<void**>(heap + 0x464) = heap + 0x448;
    *reinterpret_cast<void**>(heap + 0x4e4) = heap + 0x100;
}
}

void service_heap_lock_005dd290(void* lock) {
    assert(lock == heap + 0x100);
    ++locks;
}
void service_heap_unlock_005dd2a0(void* lock) {
    assert(lock == heap + 0x100);
    ++unlocks;
}

int main() {
    reset_heap();
    assert(service_heap_allocate_00ff9850(
               heap, nullptr, 0, 0, 0, 0, 0) == 0);
    assert(service_heap_allocate_00ff9850(
               heap, heap, 0x3f, 0, 0, 0, 0) == 0);
    assert(locks == 2 && unlocks == 2);

    auto* region = static_cast<std::uint8_t*>(VirtualAlloc(
        nullptr, 0x1000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE));
    assert(region != nullptr);
    assert(service_heap_allocate_00ff9850(
               heap, region, 0x1000, 2, 3, 1, 17) == 1);
    assert(*reinterpret_cast<void**>(heap + 0x460) == region);
    assert(*reinterpret_cast<std::uint32_t*>(region + 4) == 0x1000);
    assert(region[0x0d] == 2 && region[0x0e] == 2 && region[0x0f] == 3);
    assert(*reinterpret_cast<std::uint32_t*>(region + 0x10) == 1);
    assert(*reinterpret_cast<std::uint32_t*>(region + 0x14) == 17);
    assert(*reinterpret_cast<void**>(heap + 0x440) ==
           *reinterpret_cast<void**>(region));
    assert(VirtualFree(region, 0, MEM_RELEASE) != 0);

    reset_heap();
    assert(service_heap_allocate_00ff9850(
               heap, nullptr, 0x1000, 0, 0, 0, 0) == 1);
    region = static_cast<std::uint8_t*>(
        *reinterpret_cast<void**>(heap + 0x460));
    assert(region != nullptr && region != heap + 0x448);
    assert(*reinterpret_cast<void**>(heap + 0x440) ==
           *reinterpret_cast<void**>(region));
    assert(VirtualFree(region, 0, MEM_RELEASE) != 0);
    assert(locks == 4 && unlocks == 4);
}
