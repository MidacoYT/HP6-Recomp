// Node unlink 0x005DEF40 and conditional destruction 0x005DF520 from
// the unprotected HP6 x86 PE.
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

namespace {
template <typename T>
T& at(std::uint8_t* node, std::uintptr_t offset) {
    return *reinterpret_cast<T*>(node + offset);
}
} // namespace

void service_heap_detach_node_005def40(void* node_ptr) {
    auto* node = static_cast<std::uint8_t*>(node_ptr);
    auto* previous = at<std::uint8_t*>(node, 0x18);
    auto* next = at<std::uint8_t*>(node, 0x1c);
    at<void*>(previous, 0x1c) = next;
    at<void*>(next, 0x18) = previous;
}

void service_heap_destroy_node_005df520(void* node_ptr, void* heap,
                                        std::uint8_t flag) {
    auto* node = static_cast<std::uint8_t*>(node_ptr);
    if (node[0x0d] == 0 && (flag == 0 || node[0x0e] == 0)) return;

    using Callback = void (*)(void*, void*, void*, void*);
    auto callback = at<Callback>(node, 0x10);
    if (callback != nullptr) {
        callback(heap, node, at<void*>(node, 8), at<void*>(node, 0x14));
    } else {
#ifdef _WIN32
        VirtualFree(node, 0, MEM_RELEASE);
#endif
    }
}
