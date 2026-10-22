#include "platform/ServiceHeapConstruct.h"

#include <array>
#include <cstdint>
#include <cstring>

namespace {
std::uint8_t* active_heap;
std::uint32_t observed_request;
std::uint32_t observed_size;
std::uint8_t observed_mode;

template <typename T>
T& at(std::uint8_t* base, std::size_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}
} // namespace

void service_heap_release_callback_005ddc80(void*) {}
void service_heap_free_callback_005dd2c0(void*) {}

std::uint32_t service_heap_initialize_005dd4c0(
    void* heap, std::uint32_t request, std::uint32_t size, std::uint8_t mode) {
    active_heap = static_cast<std::uint8_t*>(heap);
    observed_request = request;
    observed_size = size;
    observed_mode = mode;
    return 1;
}

int main() {
    static_assert(sizeof(void*) == 4);
    alignas(void*) std::array<std::uint8_t, 0x518> storage{};
    std::memset(storage.data(), 0xa5, storage.size());
    auto* heap = storage.data();
    if (service_heap_construct_005dd2d0(heap, 0x34, 0x1000) != heap ||
        active_heap != heap || observed_request != 0x34 ||
        observed_size != 0x1000 || observed_mode != 1 ||
        heap[0] != 0 || at<std::uint32_t>(heap, 4) != 0 ||
        heap[0x46d] != 1 || heap[0x46e] != 0xa5 || heap[0x46f] != 0xa5 ||
        heap[0x474] != 9 || heap[0x475] != 10 ||
        at<std::uint32_t>(heap, 0x4b4) != 0x100 ||
        at<void*>(heap, 0x4b8) != reinterpret_cast<void*>(&service_heap_release_callback_005ddc80) ||
        at<void*>(heap, 0x4bc) != heap ||
        at<void*>(heap, 0x4c0) != reinterpret_cast<void*>(&service_heap_free_callback_005dd2c0) ||
        at<void*>(heap, 0x4c4) != heap ||
        at<std::uint32_t>(heap, 0x4d4) != 0x1000 ||
        at<std::uint32_t>(heap, 0x4d8) != 0x1000000 ||
        at<std::uint32_t>(heap, 0x4dc) != 0x400000 ||
        at<void*>(heap, 0x4e4) != nullptr ||
        heap[0x508] != 0xdd || heap[0x509] != 0xde ||
        heap[0x50a] != 0xcd || heap[0x50b] != 0xab || heap[0x50c] != 0xfe ||
        heap[0x514] != 0) return 1;
    for (std::size_t i = 8; i < 0x470; ++i) {
        if (i == 0x46d || i == 0x46e || i == 0x46f) continue;
        if (heap[i] != 0) return 2;
    }
    return 0;
}
