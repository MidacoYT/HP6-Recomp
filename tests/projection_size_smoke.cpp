#include "projection/FormatSize.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

int main() {
    if (hp6::projection::is_block_format_code(8) != 1 ||
        hp6::projection::is_block_format_code(15) != 1 ||
        hp6::projection::is_block_format_code(16) != 0 ||
        hp6::projection::map_format_unit(4) != 0x10 ||
        hp6::projection::map_format_unit(8) != 0x100 ||
        hp6::projection::map_format_unit(1) != 0) return 1;
    std::uint32_t geometry[3];
    hp6::projection::format_geometry(9, geometry);
    if (geometry[0] != 2 || geometry[1] != 2 || geometry[2] != 1) return 2;
    hp6::projection::format_geometry(15, geometry);
    if (geometry[0] != 8 || geometry[1] != 2 || geometry[2] != 2) return 3;
    hp6::projection::format_geometry(0, geometry);
    if (geometry[0] != 1 || geometry[1] != 1 || geometry[2] != 0) return 4;
    if (hp6::projection::extra_block_count(8, 16, 32) != 256 ||
        hp6::projection::extra_block_count(15, 16, 32) != 32) return 5;
    std::byte record[0x54] = {};
    auto set_word = [&](std::size_t offset, std::uint32_t value) {
        std::memcpy(record + offset, &value, sizeof value);
    };
    set_word(0x14, 1);
    set_word(0x18, 4);
    set_word(0x1c, 2);
    set_word(0x28, 0);
    if (hp6::projection::estimate_record_size(record) != 36) return 6;
    set_word(0x28, 1);
    if (hp6::projection::estimate_record_size(record) != 37) return 7;
    set_word(0x14, 8);
    set_word(0x18, 32);
    set_word(0x1c, 16);
    set_word(0x28, 0);
    if (hp6::projection::estimate_record_size(record) != 352) return 8;
    set_word(0x50, 1);
    if (hp6::projection::estimate_record_size(record) != 32) return 9;
    set_word(0x50, 0);
    set_word(0x18, 0);
    if (hp6::projection::estimate_record_size(record) != 0) return 10;
    std::byte variants[0x84] = {};
    auto set_variant = [&](std::size_t offset, std::uint32_t value) {
        std::memcpy(variants + offset, &value, sizeof value);
    };
    set_variant(0x14, 0x79);
    set_variant(0x54, 2);
    if (hp6::projection::estimate_variant_04_size(variants) != 48) return 11;
    set_variant(0x14, 0x7b);
    if (hp6::projection::estimate_variant_04_size(variants) != 288) return 12;
    set_variant(0x14, 0x70);
    set_variant(0x5c, 10);
    if (hp6::projection::estimate_variant_04_size(variants) != 42) return 13;
    set_variant(0x60, 1);
    if (hp6::projection::estimate_variant_04_size(variants) != 32) return 14;
    set_variant(0x7c, 3);
    set_variant(0x80, 4);
    if (hp6::projection::estimate_variant_10_size(variants) != 80) return 15;
    set_variant(0, 0x29);
    set_variant(0x0c, 12);
    if (hp6::projection::record_storage_size(variants) != 44) return 16;
    set_variant(0, 0x6f);
    if (hp6::projection::record_storage_size(variants) != 28) return 17;
    set_variant(0, 0x81);
    if (hp6::projection::record_storage_size(variants) != 0) return 18;
    return 0;
}
