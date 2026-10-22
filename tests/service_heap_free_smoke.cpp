#include <array>
#include <cstdint>

void service_heap_free_payload_005de580(void* heap, void* payload);

bool service_heap_should_release_005ddb50(void*, void*) { return false; }
void service_heap_release_pages_00ff9d00(void*, std::uint32_t) {}

namespace {
template <typename T>
T& at(std::uint8_t* base, std::size_t offset) {
    return *reinterpret_cast<T*>(base + offset);
}
}

int main() {
    static_assert(sizeof(void*) == 4);
    alignas(void*) std::array<std::uint8_t, 0x600> storage{};
    auto* heap = storage.data();
    auto* block = heap + 0x540;
    auto* payload = block + 8;
    at<std::uint32_t>(heap, 4) = 0x40;
    at<void*>(heap, 0x468) = heap + 0x5f0;
    at<std::uint32_t>(heap, 0x544) = 0x20;
    service_heap_free_payload_005de580(heap, payload);
    if (at<std::uint32_t>(heap, 4) != 0x41 ||
        at<std::uint32_t>(heap, 0x544) != 0x80000024u ||
        at<void*>(heap, 0x10) != block ||
        at<void*>(heap, 0x54c) != nullptr) return 1;
    service_heap_free_payload_005de580(heap, nullptr);
    return at<void*>(heap, 0x10) == block ? 0 : 2;
}
