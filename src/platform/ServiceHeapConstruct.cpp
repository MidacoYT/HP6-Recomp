#include "platform/ServiceHeapConstruct.h"
#include "platform/ServiceHeapInitialize.h"

#include <cstdint>
#include <cstring>

extern void service_heap_release_callback_005ddc80(void*);
extern void service_heap_free_callback_005dd2c0(void*);

namespace {
template <typename T>
T& at(std::uint8_t* base, std::uintptr_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}
} // namespace

void* service_heap_construct_005dd2d0(void* heap_ptr, std::uint32_t request,
                                       std::uint32_t size) {
    // Verified against unprotected hp6.exe 0x005DD2D0..0x005DD4BE.
    auto* heap = static_cast<std::uint8_t*>(heap_ptr);
    heap[0] = 0;
    at<std::uint32_t>(heap, 4) = 0;
    std::memset(heap + 8, 0, 0x428); // +0x08 through +0x42F.
    std::memset(heap + 0x430, 0, 0x3c); // +0x430 through +0x46B.
    heap[0x46c] = 0;
    heap[0x46d] = 1;
    at<std::uint32_t>(heap, 0x470) = 0;
    heap[0x474] = 9;
    heap[0x475] = 10;
    at<std::uint32_t>(heap, 0x478) = 0;
    at<std::uint32_t>(heap, 0x47c) = 0;
    at<std::uint32_t>(heap, 0x480) = 0;
    heap[0x484] = 0;
    at<std::uint32_t>(heap, 0x488) = 0;
    at<std::uint32_t>(heap, 0x48c) = 0;
    at<std::uint32_t>(heap, 0x490) = 0;
    at<std::uint32_t>(heap, 0x494) = 0;
    heap[0x498] = 0;
    std::memset(heap + 0x49c, 0, 0x18); // +0x49C through +0x4B3.
    at<std::uint32_t>(heap, 0x4b4) = 0x100;
    at<void*>(heap, 0x4b8) = reinterpret_cast<void*>(&service_heap_release_callback_005ddc80);
    at<void*>(heap, 0x4bc) = heap;
    at<void*>(heap, 0x4c0) = reinterpret_cast<void*>(&service_heap_free_callback_005dd2c0);
    at<void*>(heap, 0x4c4) = heap;
    at<std::uint32_t>(heap, 0x4c8) = 0;
    at<std::uint32_t>(heap, 0x4cc) = 0;
    at<std::uint32_t>(heap, 0x4d0) = 0;
    at<std::uint32_t>(heap, 0x4d4) = 0x1000;
    at<std::uint32_t>(heap, 0x4d8) = 0x1000000;
    at<std::uint32_t>(heap, 0x4dc) = 0x400000;
    heap[0x4e0] = 0;
    std::memset(heap + 0x4e4, 0, 0x24); // lock pointer and embedded lock.
    heap[0x508] = 0xdd;
    heap[0x509] = 0xde;
    heap[0x50a] = 0xcd;
    heap[0x50b] = 0xab;
    heap[0x50c] = 0xfe;
    at<std::uint32_t>(heap, 0x510) = 0;
    heap[0x514] = 0;
    service_heap_initialize_005dd4c0(heap, request, size, 1);
    return heap;
}
