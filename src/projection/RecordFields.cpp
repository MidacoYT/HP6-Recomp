#include "projection/RecordFields.h"
#include "projection/FormatTable.h"
#include <cstddef>
#include <cstring>

namespace hp6::projection {

static std::uint32_t read_word(const void* record, std::size_t offset) {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::byte*>(record) + offset, sizeof value);
    return value;
}

static void write_word(void* record, std::size_t offset, std::uint32_t value) {
    std::memcpy(static_cast<std::byte*>(record) + offset, &value, sizeof value);
}

std::uint8_t is_projection_code(std::int32_t code) {
    return static_cast<std::uint8_t>(code == 0x29 || code == 0x2a ||
                                     code == 0x2d || code == 0x2f || code == 0x32);
}

std::uint8_t is_table_projection_code(std::uint32_t code) {
    return static_cast<std::uint8_t>(format_table[code & 0xffu] != 0 &&
                                     is_projection_code(static_cast<std::int32_t>(code)) == 0);
}

std::uint32_t classify_record(const void* source) {
    const std::uint32_t code = read_word(source, 0) & 0xffu;
    if (code == 0x69) return 0x80;
    if (code == 0x6f) return 0x20;
    if (code == 0x7c) return 0x10;
    if (code == 0x81) return 1;
    if (is_projection_code(static_cast<std::int32_t>(code))) return 4;
    return is_table_projection_code(code) ? 2u : 0u;
}

std::uint8_t is_block_code(std::int32_t code) {
    return static_cast<std::uint8_t>(code == 0x60 || code == 0x61 || code == 0x62);
}

std::uint8_t is_selected_format_code(std::uint32_t code) {
    if (is_block_code(static_cast<std::int32_t>(code)) || code == 0x68u) return 0;
    const std::uint8_t value = format_table[code & 0xffu];
    return static_cast<std::uint8_t>(value == 8u || value == 15u ||
                                     value == 16u || value == 32u);
}

void copy_four_fields(const void* source, void* first, void* second,
                      void* third, void* fourth) {
    const std::uint32_t values[] = {read_word(source, 0x10), read_word(source, 0x14),
                                    read_word(source, 0x18), read_word(source, 0x1c)};
    std::memcpy(first, &values[0], 4);
    std::memcpy(second, &values[1], 4);
    std::memcpy(third, &values[2], 4);
    std::memcpy(fourth, &values[3], 4);
}

void initialize_record(void* destination) {
    std::memset(destination, 0, 0xc0);
    write_word(destination, 0x84, 0x40);
}

void* initialize_record_and_return(void* destination) {
    initialize_record(destination);
    return destination;
}

void write_projection_list(const void* first, void* destination) {
    std::byte result[0xc0];
    initialize_record(result);
    auto* current = static_cast<const std::byte*>(first);
    while (current != nullptr) {
        switch (classify_record(current)) {
        case 2: write_variant_02(current, result); break;
        case 4: write_variant_04(current, result); break;
        case 0x10: write_variant_10(current, result); break;
        case 0x20: write_variant_20(current, result); break;
        case 0x80: write_variant_80(current, result); break;
        case 0x100: write_variant_100(current, result); break;
        default: break;
        }
        const std::uint32_t next_offset = read_word(current, 4);
        current = next_offset ? current + static_cast<std::int32_t>(next_offset) : nullptr;
    }
    std::memcpy(destination, result, sizeof result);
}

// ReAgent candidates: reports/pipeline/rejected/006d7a48.cpp and peers.
// The checker rejected unnamed offsets; the x86 instructions fix their order.
void write_common_flags(const void* source, void* destination) {
    write_word(destination, 0x08, (read_word(source, 0) >> 10) & 1u);
    write_word(destination, 0x0c, (read_word(source, 0) >> 11) & 1u);
    write_word(destination, 0x10, (read_word(source, 0) >> 9) & 1u);
}

void write_variant_02(const void* source, void* destination) {
    write_common_flags(source, destination);
    write_word(destination, 0x04, read_word(destination, 0x04) | 2u);
    const std::uint32_t flags = read_word(source, 0);
    write_word(destination, 0x14, flags & 0xffu);
    write_word(destination, 0x20, read_word(source, 0x10));
    write_word(destination, 0x24, read_word(source, 0x14));
    write_word(destination, 0x18, read_word(source, 0x18));
    write_word(destination, 0x1c, read_word(source, 0x1c));
    write_word(destination, 0x28, flags >> 28);
    write_word(destination, 0x2c, (flags >> 22) & 1u);
    write_word(destination, 0x34, (flags >> 16) & 1u);
    write_word(destination, 0x38, (flags >> 17) & 1u);
    write_word(destination, 0x3c, (flags >> 18) & 1u);
    write_word(destination, 0x40, (flags >> 19) & 1u);
    write_word(destination, 0x44, (flags >> 20) & 1u);
    write_word(destination, 0x48, (flags >> 21) & 1u);
    write_word(destination, 0x4c, (flags >> 23) & 1u);
    write_word(destination, 0x30, (flags >> 24) & 1u);
}

void write_variant_04(const void* source, void* destination) {
    write_common_flags(source, destination);
    write_word(destination, 0x04, read_word(destination, 0x04) | 4u);
    write_word(destination, 0x54, read_word(source, 0) & 0xffu);
    write_word(destination, 0x5c, read_word(source, 0x18));
    write_word(destination, 0x58, (read_word(source, 0) >> 22) & 1u);
}

void write_variant_100(const void* source, void* destination) {
    write_common_flags(source, destination);
    auto* output = static_cast<std::byte*>(destination);
    output[5] |= std::byte{1};
    copy_four_fields(source, output + 0x64, output + 0x68,
                     output + 0x6c, output + 0x70);
}

void write_variant_10(const void* source, void* destination) {
    write_common_flags(source, destination);
    write_word(destination, 0x04, read_word(destination, 0x04) | 0x10u);
    write_word(destination, 0x74, read_word(source, 0x10));
    write_word(destination, 0x78, read_word(source, 0x14));
    write_word(destination, 0x7c, read_word(source, 0x18));
    write_word(destination, 0x80, read_word(source, 0x1c));
}

void write_variant_80(const void* source, void* destination) {
    write_common_flags(source, destination);
    auto* output = static_cast<std::uint8_t*>(destination);
    output[0x04] = static_cast<std::uint8_t>(output[0x04] | 0x80u);
    write_word(destination, 0x84, read_word(source, 0x0c));
}

void write_variant_20(const void* source, void* destination) {
    write_common_flags(source, destination);
    write_word(destination, 0x04, read_word(destination, 0x04) | 0x20u);
    std::uintptr_t text_address = read_word(source, 0x08);
    if (text_address != 0) {
        text_address += reinterpret_cast<std::uintptr_t>(source);
    }
    std::uint32_t length = 0;
    while (*reinterpret_cast<const char*>(text_address + length) != '\0') {
        ++length;
    }
    write_word(destination, 0x90, length);
}

}
