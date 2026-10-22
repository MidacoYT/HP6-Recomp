#include <cstdint>

// 0x005DDB00 -> 0x00FF8F90..0x00FF8FD9: large-block bin index.
std::uint32_t service_heap_large_bin_005ddb00(std::uint32_t size) {
    if ((size >> 6) <= 0x20u) return (size >> 6) + 0x38u;
    if ((size >> 9) <= 0x14u) return (size >> 9) + 0x5bu;
    if ((size >> 12) <= 0x0au) return (size >> 12) + 0x6eu;
    if ((size >> 15) <= 4u) return (size >> 15) + 0x77u;
    if ((size >> 18) <= 2u) return (size >> 18) + 0x7cu;
    return 0x7eu;
}

// 0x005DB4D0..0x005DB4F4: absolute split address for an aligned payload.
std::uint32_t service_heap_aligned_split_address_005db4d0(
    std::uint32_t alignment, std::uint32_t adjustment,
    void* block, std::uint32_t reserved) {
    const auto* bytes = static_cast<const std::uint8_t*>(block);
    const std::uint32_t block_size =
        *reinterpret_cast<const std::uint32_t*>(bytes + 4) & 0x7ffffff8u;
    const std::uint32_t target = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(block));
    return ((target + block_size - reserved + adjustment + 8u) &
            (0u - alignment)) - adjustment - 8u;
}
