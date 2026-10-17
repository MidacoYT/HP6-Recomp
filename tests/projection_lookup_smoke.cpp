#include "projection/RecordLookup.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

int main() {
    std::byte chain[0x50]{};
    const std::uint32_t link = 0x10;
    for (std::size_t offset = 0; offset < 0x40; offset += 0x10)
        std::memcpy(chain + offset + 4, &link, 4);
    chain[0] = std::byte{0x69};
    chain[0x10] = std::byte{0x29};
    chain[0x20] = std::byte{0x7c};
    chain[0x30] = std::byte{0x6f};
    chain[0x40] = std::byte{0x01};
    using namespace hp6::projection;
    if (find_variant_02(chain) != chain + 0x40) return 1;
    if (find_variant_04(chain) != chain + 0x10) return 2;
    if (find_variant_10(chain) != chain + 0x20) return 3;
    if (find_variant_20(chain) != chain + 0x30) return 4;
    if (find_record_by_kind(chain, 0x80) != chain) return 5;
    if (find_record_by_kind(chain, 0x100) != nullptr ||
        find_record_by_kind(nullptr, 2) != nullptr) return 6;
    return 0;
}
