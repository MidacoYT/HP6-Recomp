#include "projection/RecordDecode.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

int main() {
    std::byte source[0xc0] = {};
    std::byte output[0x60] = {};
    auto set_word = [](std::byte* bytes, std::size_t offset, std::uint32_t value) {
        std::memcpy(bytes + offset, &value, sizeof value);
    };
    auto get_word = [](const std::byte* bytes, std::size_t offset) {
        std::uint32_t value;
        std::memcpy(&value, bytes + offset, sizeof value);
        return value;
    };
    hp6::projection::set_relative_payload(output, output + 0x20, 11);
    if (get_word(output, 8) != 0x20 || get_word(output, 0x0c) != 11) return 1;
    hp6::projection::set_relative_payload(output, nullptr, 11);
    if (get_word(output, 8) != 0 || get_word(output, 0x0c) != 0) return 2;
    hp6::projection::link_next_record(output, output + 0x40);
    if (get_word(output, 4) != 0x40) return 3;
    hp6::projection::link_next_record(output, nullptr);
    if (get_word(output, 4) != 0) return 4;

    set_word(source, 0x08, 1);
    set_word(source, 0x0c, 0);
    set_word(source, 0x10, 1);
    set_word(source, 0x14, 1);
    set_word(source, 0x18, 4);
    set_word(source, 0x1c, 2);
    set_word(source, 0x20, 123);
    set_word(source, 0x24, 456);
    set_word(source, 0x28, 7);
    set_word(source, 0x30, 1);
    set_word(source, 0x34, 1);
    set_word(source, 0x38, 1);
    set_word(source, 0x44, 1);
    hp6::projection::decode_variant_02(source, output);
    if ((get_word(output, 0) & 0xffu) != 1 ||
        (get_word(output, 0) >> 28) != 7 ||
        (get_word(output, 0) & ((1u << 9) | (1u << 10) | (1u << 24))) !=
            ((1u << 9) | (1u << 10) | (1u << 24)) ||
        get_word(output, 0x10) != 123 ||
        get_word(output, 0x14) != 456 ||
        get_word(output, 0x18) != 4 ||
        get_word(output, 0x1c) != 2 ||
        get_word(output, 0x08) != 0x20 ||
        get_word(output, 0x0c) != 6) return 5;
    if ((get_word(output, 0) & ((1u << 16) | (1u << 17) | (1u << 20))) !=
        ((1u << 16) | (1u << 17) | (1u << 20))) return 6;

    std::memset(output, 0, sizeof output);
    set_word(source, 0x64, 11);
    set_word(source, 0x68, 22);
    set_word(source, 0x6c, 33);
    set_word(source, 0x70, 44);
    hp6::projection::decode_variant_100(source, output);
    if (get_word(output, 0) % 256 != 0x7a ||
        get_word(output, 0x10) != 11 || get_word(output, 0x14) != 22 ||
        get_word(output, 0x18) != 33 || get_word(output, 0x1c) != 44 ||
        get_word(output, 0x08) != 0x20 || get_word(output, 0x0c) != 0) return 7;
    set_word(source, 0x90, 3);
    hp6::projection::decode_variant_20(source, output);
    if (get_word(output, 0) % 256 != 0x6f ||
        get_word(output, 0x08) != 0x10 || get_word(output, 0x0c) != 4) return 8;
    std::memset(output, 0, sizeof output);
    set_word(source, 0x54, 2);
    set_word(source, 0x58, 1);
    set_word(source, 0x5c, 10);
    set_word(source, 0x60, 0);
    hp6::projection::decode_variant_04(source, output);
    if ((get_word(output, 0) & 0xffu) != 2 ||
        (get_word(output, 0) & (1u << 22)) == 0 ||
        get_word(output, 0x18) != 10 ||
        get_word(output, 0x1c) != 1 ||
        get_word(output, 0x08) != 0x20 ||
        get_word(output, 0x0c) != 10) return 9;
    set_word(source, 0x74, 7);
    set_word(source, 0x78, 8);
    set_word(source, 0x7c, 3);
    set_word(source, 0x80, 4);
    hp6::projection::decode_variant_10(source, output);
    if ((get_word(output, 0) & 0xffu) != 0x7c ||
        get_word(output, 0x10) != 7 || get_word(output, 0x14) != 8 ||
        get_word(output, 0x18) != 3 || get_word(output, 0x1c) != 4 ||
        get_word(output, 0x0c) != 48) return 10;
    std::byte clone_source[0x60] = {};
    std::byte clone_output[0x60] = {};
    set_word(clone_source, 0, 1);
    set_word(clone_source, 0x08, 0x20);
    set_word(clone_source, 0x0c, 4);
    set_word(clone_source, 0x18, 4);
    set_word(clone_source, 0x1c, 2);
    std::memcpy(clone_source + 0x20, "ABCD", 4);
    hp6::projection::clone_variant_02(clone_source, clone_output);
    if ((get_word(clone_output, 0) & 0xffu) != 1 ||
        get_word(clone_output, 0x08) != 0x20 ||
        get_word(clone_output, 0x0c) != 4 ||
        std::memcmp(clone_output + 0x20, "ABCD", 4) != 0) return 11;
    std::memset(clone_source, 0, sizeof clone_source);
    std::memset(clone_output, 0, sizeof clone_output);
    set_word(clone_source, 0, 2);
    set_word(clone_source, 0x08, 0x20);
    set_word(clone_source, 0x0c, 10);
    set_word(clone_source, 0x18, 10);
    std::memcpy(clone_source + 0x20, "ABCDEFGHIJ", 10);
    hp6::projection::clone_variant_04(clone_source, clone_output);
    if ((get_word(clone_output, 0) & 0xffu) != 2 ||
        get_word(clone_output, 0x08) != 0x20 ||
        get_word(clone_output, 0x0c) != 10 ||
        std::memcmp(clone_output + 0x20, "ABCDEFGHIJ", 10) != 0) return 12;
    std::memset(clone_source, 0, sizeof clone_source);
    std::memset(clone_output, 0, sizeof clone_output);
    set_word(clone_source, 0, 0x7a);
    set_word(clone_source, 0x08, 0x20);
    set_word(clone_source, 0x0c, 4);
    set_word(clone_source, 0x10, 11);
    set_word(clone_source, 0x14, 22);
    std::memcpy(clone_source + 0x20, "WXYZ", 4);
    hp6::projection::clone_variant_100(clone_source, clone_output);
    if ((get_word(clone_output, 0) & 0xffu) != 0x7a ||
        get_word(clone_output, 0x10) != 11 ||
        get_word(clone_output, 0x14) != 22 ||
        std::memcmp(clone_output + 0x20, "WXYZ", 4) != 0) return 13;
    std::memset(clone_source, 0, sizeof clone_source);
    std::memset(clone_output, 0, sizeof clone_output);
    set_word(clone_source, 0, 0x7c);
    set_word(clone_source, 0x08, 0x20);
    set_word(clone_source, 0x0c, 48);
    set_word(clone_source, 0x10, 7);
    set_word(clone_source, 0x14, 8);
    set_word(clone_source, 0x18, 3);
    set_word(clone_source, 0x1c, 4);
    std::memset(clone_source + 0x20, 0x5a, 48);
    hp6::projection::clone_variant_10(clone_source, clone_output);
    if ((get_word(clone_output, 0) & 0xffu) != 0x7c ||
        get_word(clone_output, 0x10) != 7 ||
        get_word(clone_output, 0x14) != 8 ||
        std::memcmp(clone_output + 0x20, clone_source + 0x20, 48) != 0)
        return 14;
    std::memset(clone_source, 0, sizeof clone_source);
    std::memset(clone_output, 0, sizeof clone_output);
    set_word(clone_source, 0, 0x69);
    set_word(clone_source, 0x08, 0x10);
    set_word(clone_source, 0x0c, 16);
    std::memset(clone_source + 0x10, 'X', 16);
    hp6::projection::clone_variant_80(clone_source, clone_output);
    if ((get_word(clone_output, 0) & 0xffu) != 0x69 ||
        get_word(clone_output, 8) != 0x10 ||
        std::strcmp(reinterpret_cast<const char*>(clone_output + 0x10), "EAGL16") != 0)
        return 15;
    std::memset(clone_source, 0, sizeof clone_source);
    std::memset(clone_output, 0, sizeof clone_output);
    set_word(clone_source, 0, 0x6f);
    set_word(clone_source, 0x08, 0x10);
    set_word(clone_source, 0x0c, 4);
    std::memcpy(clone_source + 0x10, "abc", 4);
    hp6::projection::clone_variant_20(clone_source, clone_output);
    if ((get_word(clone_output, 0) & 0xffu) != 0x6f ||
        get_word(clone_output, 0x0c) != 4 ||
        std::strcmp(reinterpret_cast<const char*>(clone_output + 0x10), "abc") != 0)
        return 16;
    return 0;
}
