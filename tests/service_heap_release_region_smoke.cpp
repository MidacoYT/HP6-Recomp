#include <array>
#include <cstdint>
#include <string>

extern void service_heap_release_region_005de540(void*, void*);

namespace {
std::array<std::uint8_t, 0x520> heap{};
std::uint8_t region{};
std::string calls;
}

void service_heap_lock_005dd290(void* lock) {
    calls += lock == heap.data() + 0x500 ? 'L' : '?';
}
void service_heap_unlock_005dd2a0(void* lock) {
    calls += lock == heap.data() + 0x500 ? 'U' : '?';
}
void service_heap_free_payload_005de580(void* owner, void* payload) {
    calls += owner == heap.data() && payload == &region ? 'F' : '?';
}

int main() {
    static_assert(sizeof(void*) == 4);
    *reinterpret_cast<void**>(heap.data() + 0x4e4) = heap.data() + 0x500;
    service_heap_release_region_005de540(heap.data(), &region);
    if (calls != "LFU") return 1;
    calls.clear();
    *reinterpret_cast<void**>(heap.data() + 0x4e4) = nullptr;
    service_heap_release_region_005de540(heap.data(), &region);
    return calls == "F" ? 0 : 2;
}
