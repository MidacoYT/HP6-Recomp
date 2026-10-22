// 0x005DDC40: shorten a free block by 0x10 and write a terminal guard.
#include <cstdint>

void service_heap_write_tail_005ddc40(void* block_ptr) {
    auto* block = static_cast<std::uint8_t*>(block_ptr);
    auto& flags = *reinterpret_cast<std::uint32_t*>(block + 4);
    const auto original_size = flags & 0x7ffffff8u;
    const auto size = original_size - 0x10u;
    flags = (flags & 0x80000007u) | size;
    *reinterpret_cast<std::uint32_t*>(block + size) = size;
    *reinterpret_cast<std::uint32_t*>(block + original_size - 0x0c) = 8;
    *reinterpret_cast<std::uint32_t*>(block + original_size - 8) = 8;
    *reinterpret_cast<std::uint32_t*>(block + original_size - 4) = 9;
}
