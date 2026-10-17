#include "math/Scalar.h"
#include "math/Vector.h"
#include "math/Transform.h"
#include "math/FloatAccess.h"
#include "stream/Metadata.h"
#include "projection/RecordFields.h"
#include <cmath>
#include <cstring>
#include <limits>

int main() {
    const float vector[4] = {3.0f, 4.0f, 0.0f, 0.0f};
    float normalized[4] = {};
    hp6::math::normalize_xyzw(vector, normalized);
    if (std::fabs(normalized[0] - 0.6f) > 0.00001f ||
        std::fabs(normalized[1] - 0.8f) > 0.00001f ||
        normalized[2] != 0.0f || normalized[3] != 0.0f) return 1;
    if (hp6::math::dot_xyz(vector, vector) != 25.0L) return 2;
    if (hp6::math::sqrt_x87(25.0f) != 5.0L) return 3;
    const float other[4] = {1.0f, 2.0f, 0.0f, -1.0f};
    if (hp6::math::different_xyz(vector, other) != 1u ||
        hp6::math::different_xyz(vector, vector) != 0u) return 4;
    float difference[4] = {};
    hp6::math::subtract_xyzw(difference, vector, other);
    if (difference[0] != 2.0f || difference[1] != 2.0f ||
        difference[2] != 0.0f || difference[3] != 1.0f) return 5;
    const float identity[16] = {1, 0, 0, 0, 0, 1, 0, 0,
                                0, 0, 1, 0, 0, 0, 0, 1};
    float transformed[3] = {};
    hp6::math::transform_translated_xyz(transformed, vector, identity);
    if (transformed[0] != 3.0f || transformed[1] != 4.0f ||
        transformed[2] != 0.0f) return 6;
    float components[3] = {1.5f, -2.0f, 3.25f};
    float* component_ptr = components;
    if (hp6::math::component0(&component_ptr) != 1.5L ||
        hp6::math::component1(&component_ptr) != -2.0L ||
        hp6::math::component2(&component_ptr) != 3.25L ||
        hp6::math::first_float(components) != 1.5L ||
        hp6::math::abs_x87(-2.0f) != 2.0L) return 7;
    std::uint32_t header[0x5c / 4] = {0x4d525453u};
    header[0x38 / 4] = 20u;
    header[0x40 / 4] = 33u;
    header[0x44 / 4] = 1234u;
    header[0x58 / 4] = 9876u;
    std::uintptr_t slot[3] = {reinterpret_cast<std::uintptr_t>(header), 77u, 88u};
    std::uintptr_t slot_address = 0, header_address = 0;
    if (hp6::stream::locate_header(slot, &slot_address, &header_address) != 0 ||
        slot_address != reinterpret_cast<std::uintptr_t>(slot) ||
        header_address != reinterpret_cast<std::uintptr_t>(header)) return 8;
    if (hp6::stream::slot_word_04(slot) != 77u ||
        hp6::stream::slot_word_08(slot) != 88u ||
        hp6::stream::header_word_44(slot) != 1234u ||
        hp6::stream::header_delta_40_38(slot) != 13 ||
        hp6::stream::header_word_58(slot) != 9876u) return 9;
    hp6::stream::set_header_words_48_4c(slot, 101u, 202u);
    if (header[0x48 / 4] != 101u || header[0x4c / 4] != 202u) return 10;

    std::uint32_t records[8] = {};
    header[0x30 / 4] = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(records));
    header[0x34 / 4] = 2u;
    if (hp6::stream::indexed_record_10(slot, 1) !=
            reinterpret_cast<std::uintptr_t>(records) ||
        hp6::stream::indexed_record_10(slot, 2) !=
            reinterpret_cast<std::uintptr_t>(records + 4) ||
        hp6::stream::indexed_record_10(slot, 0) != 0 ||
        hp6::stream::indexed_record_10(slot, 3) != 0) return 11;

    std::uint32_t descriptors[6] = {};
    header[0x28 / 4] = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(descriptors));
    header[0x2c / 4] = 2u;
    header[0x44 / 4] = 0u;
    hp6::stream::set_descriptor(slot, 1, 9, 10, 1);
    hp6::stream::set_descriptor(slot, 2, 7, 8, 2);
    if (descriptors[0] != 9u || descriptors[1] != 10u ||
        descriptors[2] != 1u || descriptors[3] != 0u) return 12;
    hp6::stream::set_descriptor(slot, 2, 0, 0, 2);
    if (descriptors[3] != 0u || descriptors[4] != 0u ||
        descriptors[5] != 2u) return 13;

    std::uint32_t entries[(0x128 / 4) * 2] = {};
    const std::uint32_t identifier = 0x12340000u;
    entries[0] = identifier;
    entries[1] = 55u;
    header[0x20 / 4] = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(entries));
    header[0x24 / 4] = 2u;
    if (hp6::stream::find_entry(reinterpret_cast<std::uintptr_t>(header), identifier) !=
            reinterpret_cast<std::uintptr_t>(entries) ||
        hp6::stream::entry_value(slot, identifier) != 55u ||
        hp6::stream::entry_value(slot, identifier + 1u) != 0u) return 14;

    slot[2] = 0;
    if (hp6::stream::is_header_inactive(slot) != 1u) return 15;
    slot[2] = 88u;
    if (hp6::stream::is_header_inactive(slot) != 0u) return 16;
    header[0] = 0;
    if (hp6::stream::locate_header(slot, &slot_address, &header_address) != 1 ||
        hp6::stream::slot_word_04(slot) != 0u ||
        hp6::stream::header_word_44(slot) != 0u ||
        hp6::stream::entry_value(slot, identifier) != 0u) return 17;

    std::byte projection_source[0x40] = {};
    std::byte projection_output[0xa0] = {};
    auto set_word = [](std::byte* bytes, std::size_t offset, std::uint32_t value) {
        std::memcpy(bytes + offset, &value, sizeof value);
    };
    auto get_word = [](const std::byte* bytes, std::size_t offset) {
        std::uint32_t value = 0;
        std::memcpy(&value, bytes + offset, sizeof value);
        return value;
    };
    const std::uint32_t flags = (1u << 10) | (1u << 9) | (1u << 22) | 0xabu;
    set_word(projection_source, 0, flags);
    set_word(projection_source, 0x08, 0x20u);
    set_word(projection_source, 0x0c, 909u);
    set_word(projection_source, 0x10, 111u);
    set_word(projection_source, 0x14, 222u);
    set_word(projection_source, 0x18, 333u);
    set_word(projection_source, 0x1c, 444u);
    std::memcpy(projection_source + 0x20, "abc", 4);
    hp6::projection::write_variant_04(projection_source, projection_output);
    if (get_word(projection_output, 0x08) != 1u ||
        get_word(projection_output, 0x0c) != 0u ||
        get_word(projection_output, 0x10) != 1u ||
        get_word(projection_output, 0x04) != 4u ||
        get_word(projection_output, 0x54) != 0xabu ||
        get_word(projection_output, 0x58) != 1u ||
        get_word(projection_output, 0x5c) != 333u) return 18;
    std::memset(projection_output, 0, sizeof projection_output);
    hp6::projection::write_variant_10(projection_source, projection_output);
    if (get_word(projection_output, 0x04) != 0x10u ||
        get_word(projection_output, 0x74) != 111u ||
        get_word(projection_output, 0x78) != 222u ||
        get_word(projection_output, 0x7c) != 333u ||
        get_word(projection_output, 0x80) != 444u) return 19;
    std::memset(projection_output, 0, sizeof projection_output);
    set_word(projection_output, 0x04, 0x12345678u);
    hp6::projection::write_variant_80(projection_source, projection_output);
    if (get_word(projection_output, 0x04) != 0x123456f8u ||
        get_word(projection_output, 0x84) != 909u) return 20;
    std::memset(projection_output, 0, sizeof projection_output);
    hp6::projection::write_variant_20(projection_source, projection_output);
    if (get_word(projection_output, 0x04) != 0x20u ||
        get_word(projection_output, 0x90) != 3u) return 21;
    std::byte full_record[0xc0];
    std::memset(full_record, 0xff, sizeof full_record);
    hp6::projection::initialize_record(full_record);
    if (get_word(full_record, 0x84) != 0x40u ||
        get_word(full_record, 0) != 0u ||
        get_word(full_record, 0xbc) != 0u) return 22;
    std::uint32_t copied[4] = {};
    hp6::projection::copy_four_fields(projection_source, &copied[0], &copied[1],
                                      &copied[2], &copied[3]);
    if (copied[0] != 111u || copied[1] != 222u ||
        copied[2] != 333u || copied[3] != 444u) return 23;
    if (hp6::projection::is_projection_code(0x29) != 1u ||
        hp6::projection::is_projection_code(0x2a) != 1u ||
        hp6::projection::is_projection_code(0x2d) != 1u ||
        hp6::projection::is_projection_code(0x2f) != 1u ||
        hp6::projection::is_projection_code(0x32) != 1u ||
        hp6::projection::is_projection_code(0x2b) != 0u ||
        hp6::projection::is_projection_code(-1) != 0u) return 24;
    hp6::projection::write_variant_02(projection_source, full_record);
    if (get_word(full_record, 0x04) != 2u ||
        get_word(full_record, 0x14) != 0xabu ||
        get_word(full_record, 0x20) != 111u ||
        get_word(full_record, 0x24) != 222u ||
        get_word(full_record, 0x18) != 333u ||
        get_word(full_record, 0x1c) != 444u ||
        get_word(full_record, 0x2c) != 1u) return 25;
    hp6::projection::write_variant_100(projection_source, full_record);
    if (get_word(full_record, 0x04) != 0x102u ||
        get_word(full_record, 0x64) != 111u ||
        get_word(full_record, 0x68) != 222u ||
        get_word(full_record, 0x6c) != 333u ||
        get_word(full_record, 0x70) != 444u) return 26;
    if (hp6::projection::initialize_record_and_return(full_record) != full_record ||
        get_word(full_record, 0x04) != 0u ||
        hp6::projection::is_block_code(0x60) != 1u ||
        hp6::projection::is_block_code(0x63) != 0u ||
        hp6::projection::is_table_projection_code(1) != 1u ||
        hp6::projection::is_table_projection_code(0x29) != 0u) return 27;
    set_word(projection_source, 0, 1u);
    if (hp6::projection::classify_record(projection_source) != 2u) return 28;
    set_word(projection_source, 0, 0x29u);
    if (hp6::projection::classify_record(projection_source) != 4u) return 29;
    set_word(projection_source, 0, 0x6fu);
    if (hp6::projection::classify_record(projection_source) != 0x20u) return 30;
    std::byte chain[0x80] = {};
    set_word(chain, 0, 0x29u | (1u << 10));
    set_word(chain, 4, 0x40u);
    set_word(chain, 0x18, 321u);
    set_word(chain, 0x40, 0x6fu);
    set_word(chain, 0x48, 0x20u);
    std::memcpy(chain + 0x60, "test", 5);
    hp6::projection::write_projection_list(chain, full_record);
    if (get_word(full_record, 0x04) != 0x24u ||
        get_word(full_record, 0x54) != 0x29u ||
        get_word(full_record, 0x5c) != 321u ||
        get_word(full_record, 0x90) != 4u ||
        get_word(full_record, 0x84) != 0x40u) return 31;
    if (hp6::projection::is_selected_format_code(2) != 1u ||
        hp6::projection::is_selected_format_code(0) != 0u ||
        hp6::projection::is_selected_format_code(0x68) != 0u ||
        hp6::projection::is_selected_format_code(0x60) != 0u) return 32;

    // cross_xyz: (1,0,0) x (0,1,0) = (0,0,1)
    {
        const float a[3] = {1.0f, 0.0f, 0.0f};
        const float b[3] = {0.0f, 1.0f, 0.0f};
        float out[3] = {9.0f, 9.0f, 9.0f};
        hp6::math::cross_xyz(a, b, out);
        if (out[0] != 0.0f || out[1] != 0.0f || out[2] != 1.0f) return 33;
    }

    // UCOMISS/LAHF parity sequence: zero vector -> 0, nonzero or NaN -> 1.
    {
        const float zero[3] = {0.0f, -0.0f, 0.0f};
        const float nonzero[3] = {0.0f, -2.0f, 0.0f};
        const float bad[3] = {1.0f, std::numeric_limits<float>::quiet_NaN(), 3.0f};
        if (hp6::math::any_nonzero_xyz(zero) != 0u) return 34;
        if (hp6::math::any_nonzero_xyz(nonzero) != 1u ||
            hp6::math::any_nonzero_xyz(bad) != 1u) return 35;
    }

    // slope_greater: a[1]*b[0] > b[1]*a[0]
    // a=(1,2), b=(1,1): left=2*1=2, right=1*1=1 -> 1
    // a=(1,1), b=(1,2): left=1*1=1, right=2*1=2 -> 0
    {
        const float a1[2] = {1.0f, 2.0f};
        const float b1[2] = {1.0f, 1.0f};
        const float a2[2] = {1.0f, 1.0f};
        const float b2[2] = {1.0f, 2.0f};
        if (hp6::math::slope_greater(a1, b1) != 1u) return 36;
        if (hp6::math::slope_greater(a2, b2) != 0u) return 37;
    }

    return 0;
}
