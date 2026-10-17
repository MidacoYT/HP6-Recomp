#include "projection/FormatSize.h"
#include "projection/FormatTable.h"
#include "projection/RecordFields.h"
#include <bit>
#include <cstddef>
#include <cstring>

namespace hp6::projection {

static std::uint32_t read_word(const void* record, std::size_t offset) {
    std::uint32_t word;
    std::memcpy(&word, static_cast<const std::byte*>(record) + offset, sizeof word);
    return word;
}

std::uint8_t is_block_format_code(std::uint32_t code) {
    return static_cast<std::uint8_t>(code >= 8 && code <= 15);
}

std::uint32_t map_format_unit(std::int32_t value) {
    if (value == 4) return 0x10;
    if (value == 8) return 0x100;
    return 0;
}

void format_geometry(std::uint32_t code, std::uint32_t output[3]) {
    output[0] = 1;
    output[1] = 1;
    output[2] = 0;
    if (code < 8 || code > 15) return;
    output[0] = 1u << ((code - 8) & 3u);
    output[1] = 2;
    output[2] = code < 12 ? 1u : 2u;
}

std::int32_t extra_block_count(std::uint32_t code, std::int32_t rows,
                               std::int32_t columns) {
    std::uint32_t geometry[3];
    format_geometry(code, geometry);
    const std::uint32_t product = static_cast<std::uint32_t>(rows) *
                                  static_cast<std::uint32_t>(columns);
    return std::bit_cast<std::int32_t>(product) /
           static_cast<std::int32_t>(geometry[0] * geometry[1]);
}

std::int32_t estimate_record_size(const void* source) {
    std::uint32_t total = 0;
    const std::int32_t level_limit = std::bit_cast<std::int32_t>(read_word(source, 0x28));
    const std::uint32_t code = read_word(source, 0x14);
    std::uint32_t width = read_word(source, 0x18);
    std::uint32_t height = read_word(source, 0x1c);
    if (read_word(source, 0x50) != 0) return 0x20;

    const std::uint8_t table_value = format_table[code & 0xffu];
    const std::uint32_t bits = table_value == 15 ? 16u : table_value;
    if (std::bit_cast<std::int32_t>(width) < 1 ||
        std::bit_cast<std::int32_t>(height) < 1) return 0;

    if (level_limit >= 0) {
        const bool align_four = is_block_code(static_cast<std::int32_t>(code)) != 0;
        const bool block_format = is_block_format_code(code) != 0;
        std::int32_t level = 0;
        do {
            std::uint32_t row_width = width;
            std::uint32_t row_count = height;
            if (align_four) {
                row_width = (width + 3u) & ~3u;
                row_count = (height + 3u) & ~3u;
            } else if (code == 0x68) {
                row_width = (width + 1u) & ~1u;
            } else if (block_format) {
                row_width = (width + 31u) & ~31u;
                row_count = (height + 15u) & ~15u;
            }
            const std::uint32_t packed = row_width * bits + 7u;
            const std::int32_t row_bytes = std::bit_cast<std::int32_t>(packed) >> 3;
            total += static_cast<std::uint32_t>(row_bytes) * row_count;
            if (block_format) {
                total += static_cast<std::uint32_t>(extra_block_count(
                    code, static_cast<std::int32_t>(row_count),
                    static_cast<std::int32_t>(row_width)));
            }
            if (width == 1 && height == 1) break;
            width = static_cast<std::uint32_t>(std::bit_cast<std::int32_t>(width) >> 1);
            height = static_cast<std::uint32_t>(std::bit_cast<std::int32_t>(height) >> 1);
            if (width < 1) width = 1;
            if (height < 1) height = 1;
            ++level;
        } while (level <= level_limit);
    }
    return std::bit_cast<std::int32_t>(total + 0x20u);
}

std::int32_t estimate_variant_04_size(const void* source) {
    const std::uint32_t code = read_word(source, 0x14);
    const std::uint32_t multiplier = format_table[read_word(source, 0x54) & 0xffu];
    std::uint32_t count = read_word(source, 0x5c);
    if ((count == 0 && code == 0x79u) || code == 0x7bu) {
        count = map_format_unit(format_table[code & 0xffu]);
    }
    std::uint32_t size = 0;
    if (read_word(source, 0x60) == 0) {
        const std::uint32_t product = count * multiplier;
        size = static_cast<std::uint32_t>(std::bit_cast<std::int32_t>(product) / 8);
    }
    return std::bit_cast<std::int32_t>(size + 0x20u);
}

std::int32_t estimate_variant_10_size(const void* source) {
    const std::uint32_t product = read_word(source, 0x80) *
                                  read_word(source, 0x7c) * 4u;
    return std::bit_cast<std::int32_t>(product + 0x20u);
}

std::uint32_t estimate_collection_size(const void* source) {
    const std::uint32_t flags = read_word(source, 4);
    std::uint32_t size = (flags & 2u) ?
        static_cast<std::uint32_t>(estimate_record_size(source)) : 0u;
    if (flags & 4u) {
        if (size != 0 && read_word(source, 0x60) == 0) {
            size = (size + 15u) & ~15u;
        }
        size += static_cast<std::uint32_t>(estimate_variant_04_size(source));
    }
    if (flags & 0x100u) {
        size = (size == 0 ? 0u : (size + 15u) & ~15u) + 0x20u;
    }
    if (flags & 0x10u) {
        size = (size == 0 ? 0u : (size + 15u) & ~15u) +
               static_cast<std::uint32_t>(estimate_variant_10_size(source));
    }
    if (flags & 0x80u) {
        const std::uint32_t aligned = size == 0 ? 0u : ((size - 1u) & ~15u) + 0x10u;
        size = aligned + 0x10u + read_word(source, 0x84);
    }
    if (flags & 0x20u) {
        const std::uint32_t aligned = size == 0 ? 0u : ((size - 1u) & ~15u) + 0x10u;
        size = aligned + 0x11u + read_word(source, 0x90);
    }
    return size;
}

std::uint32_t record_storage_size(const void* source) {
    for (;;) {
        switch (classify_record(source)) {
        case 1: return 0;
        case 2:
        case 4:
        case 0x10: return read_word(source, 0x0c) + 0x20u;
        case 0x20:
        case 0x80: return read_word(source, 0x0c) + 0x10u;
        case 0x100: continue;
        default: return 0;
        }
    }
}

std::uint32_t fixed_clip_storage_size() { return 0x20u; }

std::uint32_t comment_storage_size(const void* source) {
    return read_word(source, 0x90) + 0x11u;
}

std::uint32_t eagl_storage_size(const void* source) {
    return read_word(source, 0x84) + 0x10u;
}

}
