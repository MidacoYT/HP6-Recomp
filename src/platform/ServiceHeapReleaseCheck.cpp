// 0x005DDB50: find whether a block end lies in the final 0x10 bytes of
// a tracked virtual allocation. The x86 caller reads its low return byte.
#include <cstdint>

namespace {
std::uint32_t word(const std::uint8_t* p) {
    return *reinterpret_cast<const std::uint32_t*>(p);
}
const std::uint8_t* pointer(const std::uint8_t* p) {
    return *reinterpret_cast<std::uint8_t* const*>(p);
}
} // namespace

bool service_heap_should_release_005ddb50(void* end_ptr, void* heap_ptr) {
    auto* end = static_cast<const std::uint8_t*>(end_ptr);
    auto* heap = static_cast<const std::uint8_t*>(heap_ptr);
    if ((word(end + 4) & 0x7ffffff8u) >= 0x10u) return false;

    for (auto* node = pointer(heap + 0x460); node != heap + 0x448;
         node = pointer(node + 0x18)) {
        const auto address = reinterpret_cast<std::uintptr_t>(node);
        const auto target = reinterpret_cast<std::uintptr_t>(end);
        const auto limit = address + word(node + 4);
        if (address <= target && target < limit) {
            return limit - 0x10u <= target;
        }
    }
    return false;
}
