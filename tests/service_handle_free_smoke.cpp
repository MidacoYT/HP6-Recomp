#include <array>
#include <cstdint>
#include <string>

extern void service_handle_free_005db8d0(void*, std::uint32_t);
void* DAT_00cef6b0 = nullptr;

namespace {
std::string calls;
void* expected_block;
}

void service_heap_lock_005dd290(void* lock) {
    calls += lock == static_cast<std::uint8_t*>(DAT_00cef6b0) + 0x500 ? 'L' : '?';
}
void service_heap_unlock_005dd2a0(void* lock) {
    calls += lock == static_cast<std::uint8_t*>(DAT_00cef6b0) + 0x500 ? 'U' : '?';
}
void service_heap_free_payload_005de580(void* heap, void* block) {
    calls += heap == DAT_00cef6b0 && block == expected_block ? 'F' : '?';
}

int main() {
    static_assert(sizeof(void*) == 4);
    alignas(void*) std::array<std::uint8_t, 0x520> heap{};
    DAT_00cef6b0 = heap.data();
    expected_block = heap.data() + 0x100;
    service_handle_free_005db8d0(nullptr, 1);
    if (!calls.empty()) return 1;
    service_handle_free_005db8d0(expected_block, 0);
    if (calls != "F") return 2;
    calls.clear();
    *reinterpret_cast<void**>(heap.data() + 0x4e4) = heap.data() + 0x500;
    service_handle_free_005db8d0(expected_block, 0);
    return calls == "LFU" ? 0 : 3;
}
