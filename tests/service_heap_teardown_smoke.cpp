#include "platform/ServiceHeapTeardown.h"

#include <array>
#include <cstdint>
#include <cstring>
#include <string>

void* DAT_00cef724 = nullptr;

namespace {
std::string calls;
std::uint8_t* active_heap = nullptr;
void* expected_payload = nullptr;

template <typename T>
T& at(std::uint8_t* base, std::size_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}

struct CallbackNode {
    void (*callback)(void*, std::uint32_t, void*);
    void* context;
    CallbackNode* next;
};

void on_shutdown(void* heap, std::uint32_t mode, void* context) {
    if (heap == active_heap && mode == 0 && context == active_heap + 7) calls += 'C';
    else calls += '?';
}
} // namespace

void service_heap_lock_005dd290(void* lock) {
    calls += lock == active_heap + 0x500 ? 'L' : '?';
}
void service_heap_prepare_release_after_flag_005dec90(void* heap) {
    calls += heap == active_heap ? 'P' : '?';
}
void service_heap_free_payload_005de580(void* heap, void* payload) {
    calls += heap == active_heap && payload == expected_payload ? 'F' : '?';
    at<void*>(active_heap, 0x4a8) = active_heap + 0x49c;
}
void service_heap_detach_node_005def40(void*) { calls += 'D'; }
void service_heap_destroy_node_005df520(void* node, void* heap, std::uint8_t flag) {
    calls += node == active_heap + 0x480 && heap == active_heap && flag == 1 ? 'N' : '?';
    at<void*>(active_heap, 0x464) = active_heap + 0x448;
}
void service_heap_unlock_005dd2a0(void* lock) {
    calls += lock == active_heap + 0x500 ? 'U' : '?';
}
void service_heap_free_lock_005dd280(void* lock) {
    calls += lock == active_heap + 0x500 ? 'X' : '?';
}

int main() {
    static_assert(sizeof(void*) == 4);
    alignas(void*) std::array<std::uint8_t, 0x520> storage{};
    active_heap = storage.data();
    active_heap[0] = 1;
    active_heap[0x514] = 1;
    at<std::uint32_t>(active_heap, 4) = 1;
    at<void*>(active_heap, 0x4e4) = active_heap + 0x500;
    at<void*>(active_heap, 0x4a8) = active_heap + 0x420;
    at<std::uint32_t>(active_heap, 0x420) = 0x10;
    expected_payload = active_heap + 0x418;
    at<void*>(active_heap, 0x464) = active_heap + 0x480;
    CallbackNode callback{on_shutdown, active_heap + 7, nullptr};
    DAT_00cef724 = &callback;

    if (teardown_service_heap_005dd6b0(active_heap) != 1 ||
        calls != "CLPFDNUX" || active_heap[0] != 0 || active_heap[0x514] != 0 ||
        at<void*>(active_heap, 0x4e4) != nullptr ||
        at<void*>(active_heap, 0x440) != active_heap + 0x30 ||
        at<std::uint32_t>(active_heap, 0x468) != 0 || active_heap[0x46c] != 0)
        return 1;

    calls.clear();
    DAT_00cef724 = nullptr;
    if (teardown_service_heap_005dd6b0(active_heap) != 1 || !calls.empty()) return 2;
    return 0;
}
