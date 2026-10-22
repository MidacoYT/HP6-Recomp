#include <array>
#include <cstdint>

extern void service_heap_dispatch_005dd830_1(void*);
extern void service_heap_dispatch_005dd830_2(void*, std::uint32_t);

int main() {
    std::array<std::uint8_t, 0x470> heap{};
    heap[0x46d] = 0xa5;
    service_heap_dispatch_005dd830_1(heap.data());
    if (heap[0x46d] != 0xa5) return 1;
    service_heap_dispatch_005dd830_2(heap.data(), 0);
    if (heap[0x46d] != 0) return 2;
    service_heap_dispatch_005dd830_2(heap.data(), 7);
    return heap[0x46d] == 1 ? 0 : 3;
}
