#include "projection/CollectionSizing.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace {
void set_word(std::byte* base, std::size_t offset, std::uint32_t value) {
    std::memcpy(base + offset, &value, 4);
}
}

int main() {
    alignas(128) std::byte chain[0x50]{};
    set_word(chain, 0, 0x6f); // comment-like record
    set_word(chain, 4, 0x20); // relative link
    set_word(chain, 0x0c, 4);
    set_word(chain, 0x20, 0x69); // EAGL-like record
    set_word(chain, 0x2c, 5);
    std::byte input[0xa0]{};
    set_word(input, 4, 0x20);
    set_word(input, 0x90, 32);
    using hp6::projection::measure_collection_update;
    if (measure_collection_update(chain, input, 1) != 53) return 1;
    if (measure_collection_update(chain, input, 2) != 21) return 2;
    if (measure_collection_update(chain, input, 3) != 85) return 3;
    if (measure_collection_update(nullptr, input, 1) != 49) return 4;
    set_word(input, 4, 0x80);
    set_word(input, 0x84, 9);
    if (measure_collection_update(chain, input, 2) != 20) return 5;
    if (measure_collection_update(chain, input, 3) != 57) return 6;
    using hp6::projection::measure_record_replacement;
    set_word(input, 0, 0x6f);
    set_word(input, 0x0c, 32);
    if (measure_record_replacement(chain, input, 1) != 101) return 7;
    if (measure_record_replacement(chain, input, 2) != 21) return 8;
    if (measure_record_replacement(chain, input, 3) != 69) return 9;
    set_word(input, 0, 0x69);
    set_word(input, 0x0c, 9);
    if (measure_record_replacement(chain, input, 1) != 89) return 10;
    if (measure_record_replacement(chain, input, 2) != 20) return 11;
    if (measure_record_replacement(chain, input, 3) != 57) return 12;
    return 0;
}
