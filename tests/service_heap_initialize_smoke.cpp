#include "platform/ServiceHeapInitialize.h"

#include <array>
#include <cstdint>
#include <string>
#include <windows.h>

void* DAT_00cef724 = nullptr;

namespace {
std::string calls;
std::uint8_t* active_heap;

template <typename T>
T& at(std::uint8_t* base, std::size_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}

struct CallbackNode {
    std::uint32_t (*callback)(void*, std::uint32_t, void*);
    void* context;
    CallbackNode* next;
};

std::uint32_t callback(void* heap, std::uint32_t mode, void* context) {
    calls += heap == active_heap && mode == 1 && context == active_heap + 3 ? 'C' : '?';
    return 0x12345678;
}
} // namespace

void service_heap_lock_005dd290(void* lock) {
    calls += lock == active_heap + 0x4e8 ? 'L' : '?';
}
void service_heap_unlock_005dd2a0(void* lock) {
    calls += lock == active_heap + 0x4e8 ? 'U' : '?';
}
std::uint32_t service_heap_allocate_00ff9850(
    void* heap, void* region, std::uint32_t size, std::uint8_t mode,
    std::uint32_t a, std::uint32_t b, std::uint32_t c) {
    calls += heap == active_heap && region == reinterpret_cast<void*>(1) &&
                     size == 0x80 && mode == 2 &&
                     a == 0 && b == 0 && c == 0 ? 'A' : '?';
    return 0xabcdef00;
}

int main() {
    static_assert(sizeof(void*) == 4);
    alignas(void*) std::array<std::uint8_t, 0x520> storage{};
    active_heap = storage.data();
    CallbackNode node{callback, active_heap + 3, nullptr};
    DAT_00cef724 = &node;
    SYSTEM_INFO info{};
    GetSystemInfo(&info);

    const auto result = service_heap_initialize_005dd4c0(active_heap, 1, 0x80, 2);
    if (result != 0x12345601 || calls != "LUAC" || active_heap[0] != 1 ||
        active_heap[0x514] != 1 || at<std::uint32_t>(active_heap, 4) != 0x48 ||
        at<std::uint32_t>(active_heap, 0x490) != 0x10000 ||
        at<std::uint32_t>(active_heap, 0x494) != 0x20000 ||
        at<std::uint32_t>(active_heap, 0x4c8) != 0x40000 ||
        at<std::uint32_t>(active_heap, 0x4d4) != info.dwPageSize ||
        at<void*>(active_heap, 0x440) != active_heap + 0x30 ||
        at<void*>(active_heap, 0x460) != active_heap + 0x448 ||
        at<void*>(active_heap, 0x464) != active_heap + 0x448 ||
        at<void*>(active_heap, 0x4a4) != active_heap + 0x49c ||
        at<void*>(active_heap, 0x4a8) != active_heap + 0x49c ||
        at<void*>(active_heap, 0x4d0) != active_heap + 0x30)
        return 1;
    if (at<void*>(active_heap, 0x4e4) != active_heap + 0x4e8 ||
        at<std::uint32_t>(active_heap, 0x500) != 0) return 4;
    for (std::uint32_t i = 0; i < 0x7f; ++i) {
        auto* entry = active_heap + 0x30 + 8 * i;
        if (at<void*>(entry, 8) != entry || at<void*>(entry, 12) != entry) return 2;
    }
    calls.clear();
    if (service_heap_initialize_005dd4c0(active_heap, 0, 0, 0) != 1 ||
        !calls.empty()) return 3;
    DeleteCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(active_heap + 0x4e8));
    return 0;
}
