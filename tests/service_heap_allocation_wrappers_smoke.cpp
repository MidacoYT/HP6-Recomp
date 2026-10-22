#include <array>
#include <cstdint>
#include <string>

extern void* service_heap_allocate_locked_005dddb0(
    void*, std::uint32_t, std::uint32_t);
extern std::uint32_t service_heap_usable_end_offset_005dede0(void*, void*);

namespace {
std::string calls;
std::array<std::uint8_t, 0x520> heap{};
std::array<std::uint8_t, 64> block{};
std::uint32_t requested_size = 0;
std::uint32_t requested_flags = 0;
}

void service_heap_lock_005dd290(void* lock) {
    calls += lock == heap.data() + 0x500 ? 'L' : '?';
}
void service_heap_unlock_005dd2a0(void* lock) {
    calls += lock == heap.data() + 0x500 ? 'U' : '?';
}
void* service_heap_allocate_core_005dde30(
    void* owner, std::uint32_t size, std::uint32_t flags) {
    calls += owner == heap.data() ? 'A' : '?';
    requested_size = size;
    requested_flags = flags;
    return block.data() + 8;
}

int main() {
    static_assert(sizeof(void*) == 4);
    *reinterpret_cast<void**>(heap.data() + 0x4e4) = heap.data() + 0x500;
    if (service_heap_allocate_locked_005dddb0(heap.data(), 44, 7) !=
            block.data() + 8 || calls != "LAU" ||
        requested_size != 44 || requested_flags != 7) return 1;
    calls.clear();
    *reinterpret_cast<std::uint32_t*>(block.data() + 4) = 26; // length 24, flag 2
    if (service_heap_usable_end_offset_005dede0(
            heap.data(), block.data() + 8) != 16 || calls != "LU") return 2;
    calls.clear();
    *reinterpret_cast<std::uint32_t*>(block.data() + 4) = 24; // length 24, no flag 2
    block[8 + 24 - 4] = 1;
    if (service_heap_usable_end_offset_005dede0(
            heap.data(), block.data() + 8) != 20 || calls != "LU") return 3;
    calls.clear();
    block[8 + 24 - 4] = 0;
    if (service_heap_usable_end_offset_005dede0(
            heap.data(), block.data() + 8) != 0 || calls != "LU") return 4;
    calls.clear();
    if (service_heap_usable_end_offset_005dede0(heap.data(), nullptr) != 0 ||
        calls != "LU") return 5;
    return 0;
}
