#include <cstdint>

// 0x005DD830 jump-table slots used at 0x00FF8C6C..0x00FF8C8E.
// Slot 1 points to 0x005DDAAE, the shared return epilogue.
void service_heap_dispatch_005dd830_1(void*) {}

// Slot 2 points to 0x005DD8D3. The register argument ESI becomes one byte.
void service_heap_dispatch_005dd830_2(void* heap, std::uint32_t value) {
    auto* bytes = static_cast<std::uint8_t*>(heap);
    bytes[0x46d] = value != 0 ? 1 : 0;
}
