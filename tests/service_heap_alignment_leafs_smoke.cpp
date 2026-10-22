#include <cassert>
#include <cstdint>

extern std::uint32_t service_heap_large_bin_005ddb00(std::uint32_t);
extern std::uint32_t service_heap_aligned_split_address_005db4d0(
    std::uint32_t, std::uint32_t, void*, std::uint32_t);

int main() {
    assert(service_heap_large_bin_005ddb00(0) == 0x38);
    assert(service_heap_large_bin_005ddb00(0x800) == 0x58);
    assert(service_heap_large_bin_005ddb00(0x840) == 0x5f);
    assert(service_heap_large_bin_005ddb00(0xffffffffu) == 0x7e);

    std::uint8_t block[256]{};
    *reinterpret_cast<std::uint32_t*>(block + 4) = 0x85;
    const std::uint32_t split_address = service_heap_aligned_split_address_005db4d0(
        16, 8, block, 0x30);
    const std::uint32_t block_address = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(block));
    assert(((split_address + 8 + 8) & 15u) == 0);
    assert(split_address >= block_address);
    assert(split_address < block_address + 0x80);
}
