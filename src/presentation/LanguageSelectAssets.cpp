#include "presentation/LanguageSelectAssets.h"

#include <algorithm>
#include <cstring>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

namespace hp6::presentation {
namespace {
using Bytes = std::span<const std::uint8_t>;

std::uint32_t word(Bytes data, std::size_t offset) {
    if (offset > data.size() || data.size() - offset < 4) {
        throw std::runtime_error("LanguageSelect: truncated word");
    }
    return std::uint32_t(data[offset]) | std::uint32_t(data[offset + 1]) << 8 |
           std::uint32_t(data[offset + 2]) << 16 |
           std::uint32_t(data[offset + 3]) << 24;
}

std::uint16_t half(Bytes data, std::size_t offset) {
    if (offset > data.size() || data.size() - offset < 2) {
        throw std::runtime_error("LanguageSelect: truncated halfword");
    }
    return std::uint16_t(data[offset]) |
           std::uint16_t(data[offset + 1]) << 8;
}

float real(Bytes data, std::size_t offset) {
    const auto bits = word(data, offset);
    float value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

std::string tag(Bytes data, std::size_t offset) {
    if (offset > data.size() || data.size() - offset < 4) {
        throw std::runtime_error("LanguageSelect: truncated chunk tag");
    }
    return {char(data[offset + 3]), char(data[offset + 2]),
            char(data[offset + 1]), char(data[offset])};
}

struct Chunk {
    std::string name;
    std::size_t body;
    std::size_t end;
};

std::vector<Chunk> chunks(Bytes data, std::size_t first, std::size_t limit) {
    std::vector<Chunk> result;
    for (auto offset = first; offset < limit;) {
        if (limit - offset < 8) {
            throw std::runtime_error("LanguageSelect: truncated chunk header");
        }
        const auto size = word(data, offset + 4);
        if (size < 8 || size > limit - offset) {
            throw std::runtime_error("LanguageSelect: invalid chunk size");
        }
        result.push_back({tag(data, offset), offset + 8, offset + size});
        offset += size;
    }
    return result;
}

std::uint32_t name_hash(std::string_view name) {
    std::uint32_t value = 0;
    for (unsigned char byte : name) {
        if (byte == '.') break; // hashes use the basename without extension
        if (byte >= 'A' && byte <= 'Z') byte = byte - 'A' + 'a';
        value = (value << 4) + byte;
        const auto high = value & 0xf0000000u;
        if (high) value ^= high >> 24;
        value &= ~high;
    }
    return value;
}

std::uint32_t channel_hash(std::string_view name) {
    std::uint32_t value = 0;
    for (unsigned char byte : name) {
        if (byte >= 'A' && byte <= 'Z') byte = byte - 'A' + 'a';
        value = (value << 4) + byte;
        const auto high = value & 0xf0000000u;
        if (high) value ^= high >> 24;
        value &= ~high;
    }
    return value;
}

struct Asset {
    std::uint32_t type;
    Bytes data;
};

using AssetMap = std::unordered_map<std::uint32_t, Asset>;

AssetMap read_asset_map(Bytes archive) {
    const auto root = chunks(archive, 0, archive.size());
    if (root.size() != 1 || root[0].name != "STRM") {
        throw std::runtime_error("LanguageSelect: expected one STRM root");
    }
    const auto top = chunks(archive, root[0].body, root[0].end);
    std::size_t data_begin = 0, data_end = 0;
    for (const auto& chunk : top) {
        if (chunk.name == "DATA") {
            data_begin = chunk.body;
            data_end = chunk.end;
        }
    }
    if (data_end == 0) {
        throw std::runtime_error("LanguageSelect: DATA chunk missing");
    }
    AssetMap assets;
    for (const auto& group : top) {
        if (group.name != "AGRP") continue;
        for (const auto& chunk : chunks(archive, group.body, group.end)) {
            if (chunk.name != "ASET") continue;
            if (chunk.end - chunk.body != 32 && chunk.end - chunk.body != 40) {
                throw std::runtime_error("LanguageSelect: invalid ASET size");
            }
            const auto hash = word(archive, chunk.body);
            const auto type = word(archive, chunk.body + 4);
            const auto offset = word(archive, chunk.body + 16);
            const auto size = word(archive, chunk.body + 20);
            if (offset < data_begin || offset > data_end ||
                size > data_end - offset) {
                throw std::runtime_error("LanguageSelect: asset outside DATA");
            }
            assets.emplace(hash, Asset{type, archive.subspan(offset, size)});
        }
    }
    return assets;
}

Bytes find_hybrid(const AssetMap& assets, std::string_view name) {
    const auto found = assets.find(name_hash(name));
    if (found == assets.end() || found->second.type != 0x06c5b0b9u) {
        throw std::runtime_error("LanguageSelect: missing hybrid asset " +
                                 std::string(name));
    }
    return found->second.data;
}

Bytes find_entity(const AssetMap& assets, std::string_view name,
                  std::string_view class_name) {
    const auto data = find_hybrid(assets, name);
    if (word(data, 0x1c) != name_hash(class_name)) {
        throw std::runtime_error("LanguageSelect: wrong entity class for " +
                                 std::string(name));
    }
    return data;
}

LanguageSelectFontAsset read_font(const AssetMap& assets) {
    const auto found = assets.find(name_hash("LS_Ascender_Uni_211"));
    if (found == assets.end() || found->second.type != 0x7ba6u) {
        throw std::runtime_error("LanguageSelect: font asset missing");
    }
    const auto font = found->second.data;
    if (font.size() < 32 || std::memcmp(font.data(), "FntF", 4) != 0) {
        throw std::runtime_error("LanguageSelect: invalid FntF header");
    }
    const auto count = half(font, 10);
    const auto flags = word(font, 12);
    const auto glyph_start = word(font, 20);
    const auto texture_start = word(font, 28);
    if (count > 4096 || (flags & 0x40000u) == 0 ||
        glyph_start > font.size() || count > (font.size() - glyph_start) / 16) {
        throw std::runtime_error("LanguageSelect: invalid FntF glyph table");
    }
    LanguageSelectFontAsset result;
    result.metric_height = word(font, 0x24);
    if (result.metric_height == 0) {
        throw std::runtime_error("LanguageSelect: invalid FntF height metric");
    }
    for (std::size_t index = 0; index < count; ++index) {
        const auto offset = glyph_start + index * 16;
        const auto codepoint = half(font, offset);
        const LanguageSelectFontGlyph glyph{
            font[offset + 2], font[offset + 3],
            static_cast<std::int8_t>(font[offset + 10]),
            half(font, offset + 4),
            half(font, offset + 6), half(font, offset + 14)};
        if (!result.glyphs.emplace(codepoint, glyph).second) {
            throw std::runtime_error("LanguageSelect: duplicate font glyph");
        }
    }
    if (texture_start > font.size() || font.size() - texture_start < 16 ||
        font[texture_start] != 123) {
        throw std::runtime_error("LanguageSelect: invalid FntF atlas header");
    }
    result.atlas_width = half(font, texture_start + 4);
    result.atlas_height = half(font, texture_start + 6);
    if (result.atlas_width == 0 || result.atlas_height == 0 ||
        result.atlas_width > 4096 || result.atlas_height > 4096) {
        throw std::runtime_error("LanguageSelect: invalid FntF atlas dimensions");
    }
    const auto alpha_start = texture_start + 16;
    const std::size_t pixel_count =
        std::size_t(result.atlas_width) * result.atlas_height;
    if (pixel_count > font.size() - alpha_start) {
        throw std::runtime_error("LanguageSelect: truncated FntF atlas");
    }
    result.alpha.assign(font.begin() + alpha_start,
                        font.begin() + alpha_start + pixel_count);
    for (const auto& [codepoint, glyph] : result.glyphs) {
        if (std::uint32_t(glyph.atlas_u) + glyph.width > result.atlas_width ||
            std::uint32_t(glyph.atlas_v) + glyph.height > result.atlas_height) {
            throw std::runtime_error("LanguageSelect: glyph outside FntF atlas");
        }
    }
    return result;
}

LanguageSelectTextureAsset read_texture(const AssetMap& assets,
                                        std::string_view name) {
    const auto found = assets.find(name_hash(name));
    if (found == assets.end() || found->second.type != 0x7ad1u) {
        throw std::runtime_error("LanguageSelect: missing SHPI texture " +
                                 std::string(name));
    }
    const auto shpi = found->second.data;
    if (shpi.size() < 24 || std::memcmp(shpi.data(), "SHPI", 4) != 0 ||
        word(shpi, 8) != 1) {
        throw std::runtime_error("LanguageSelect: invalid SHPI container");
    }
    const auto offset = word(shpi, 20);
    if (offset > shpi.size() || shpi.size() - offset < 32) {
        throw std::runtime_error("LanguageSelect: invalid SHPI bitmap offset");
    }
    LanguageSelectTextureAsset result;
    result.format = shpi[offset];
    result.width = half(shpi, offset + 4);
    result.height = half(shpi, offset + 6);
    if (result.format < 0x60 || result.format > 0x62 ||
        result.width == 0 || result.height == 0) {
        throw std::runtime_error("LanguageSelect: unsupported SHPI bitmap");
    }
    const std::size_t block_size = result.format == 0x60 ? 8 : 16;
    const std::size_t mip0_size =
        ((std::size_t(result.width) + 3) / 4) *
        ((std::size_t(result.height) + 3) / 4) * block_size;
    const auto declared = std::uint32_t(shpi[offset + 1]) |
        std::uint32_t(shpi[offset + 2]) << 8 |
        std::uint32_t(shpi[offset + 3]) << 16;
    std::size_t chain_size = mip0_size;
    std::size_t level_width = result.width;
    std::size_t level_height = result.height;
    result.mip_levels = 1;
    while (level_width != 1 || level_height != 1) {
        level_width = std::max<std::size_t>(1, level_width / 2);
        level_height = std::max<std::size_t>(1, level_height / 2);
        const auto next_size = ((level_width + 3) / 4) *
                               ((level_height + 3) / 4) * block_size;
        if (chain_size + next_size > declared ||
            chain_size + next_size > shpi.size() - offset - 32) break;
        chain_size += next_size;
        ++result.mip_levels;
    }
    if (declared < chain_size || chain_size > shpi.size() - offset - 32) {
        throw std::runtime_error("LanguageSelect: truncated SHPI mip chain");
    }
    result.mip0_blocks.assign(shpi.begin() + offset + 32,
                              shpi.begin() + offset + 32 + mip0_size);
    result.mip_chain_blocks.assign(shpi.begin() + offset + 32,
                                   shpi.begin() + offset + 32 + chain_size);
    return result;
}

std::vector<std::uint32_t> children(Bytes entity) {
    const auto count = word(entity, 0x0c);
    const auto start = word(entity, 0x10);
    if (start > entity.size() || count > (entity.size() - start) / 20) {
        throw std::runtime_error("LanguageSelect: invalid child table");
    }
    std::vector<std::uint32_t> result;
    result.reserve(count);
    for (std::uint32_t index = 0; index < count; ++index) {
        result.push_back(word(entity, start + index * 20));
    }
    return result;
}

void expect_children(Bytes entity, std::span<const std::uint32_t> expected) {
    const auto actual = children(entity);
    if (!std::equal(actual.begin(), actual.end(), expected.begin(), expected.end())) {
        throw std::runtime_error("LanguageSelect: unexpected child hierarchy");
    }
}

std::uint32_t property_word(Bytes entity, std::size_t entry,
                            std::uint32_t property_hash) {
    // These particular hybrid properties are 12-byte records: hash, width,
    // and a 4-byte value (which can itself be a string offset).
    if (word(entity, entry) != property_hash || word(entity, entry + 4) != 4) {
        throw std::runtime_error("LanguageSelect: unexpected property");
    }
    return word(entity, entry + 8);
}

std::string property_string(Bytes entity, std::size_t entry,
                            std::uint32_t property_hash) {
    const auto offset = property_word(entity, entry, property_hash);
    if (offset >= entity.size()) {
        throw std::runtime_error("LanguageSelect: string outside entity");
    }
    const auto* begin = reinterpret_cast<const char*>(entity.data() + offset);
    const auto* end = static_cast<const char*>(
        std::memchr(begin, 0, entity.size() - offset));
    if (!end) {
        throw std::runtime_error("LanguageSelect: unterminated string property");
    }
    return {begin, end};
}

void read_button_animation(Bytes entity, LanguageSelectTextAsset& result) {
    // The property 95 override group follows the string property group in
    // these TextButtons. Native 0x539a00 zeroes all nine fields before the
    // serialized overrides are applied.
    constexpr std::size_t group = 0xc0;
    constexpr std::uint32_t animation_type_hash = 0x0701587eu;
    constexpr std::uint32_t bank_field_hash = 0x0aaf45f5u;
    constexpr std::array<std::uint32_t, 9> clip_hashes = {
        0x0956a1beu, 0x0651c47eu, 0x0cad03a5u,
        0x0f3c5b6eu, 0x09165b3eu, 0x06e06255u,
        0x096e8c3eu, 0x0aa78b1eu, 0x0ac36a15u};
    if (word(entity, group) != animation_type_hash) {
        throw std::runtime_error("LanguageSelect: animation property 95 missing");
    }
    const auto count = word(entity, group + 4);
    if (count > 10) {
        throw std::runtime_error("LanguageSelect: invalid animation override count");
    }
    std::array<bool, 10> seen{};
    std::size_t entry = group + 8;
    for (std::uint32_t index = 0; index < count; ++index) {
        const auto hash = word(entity, entry);
        const auto size = word(entity, entry + 4);
        if (size > entity.size() - std::min(entry + 8, entity.size())) {
            throw std::runtime_error("LanguageSelect: truncated animation override");
        }
        if (hash == bank_field_hash && size == 16 && !seen[0]) {
            result.animation_bank_hash = word(entity, entry + 8);
            seen[0] = true;
        } else {
            const auto field = std::find(clip_hashes.begin(), clip_hashes.end(), hash);
            if (field == clip_hashes.end() || size != 4) {
                throw std::runtime_error("LanguageSelect: unknown animation override");
            }
            const auto state = std::size_t(field - clip_hashes.begin());
            if (seen[state + 1]) {
                throw std::runtime_error("LanguageSelect: repeated animation override");
            }
            result.animation_clips[state] = property_string(entity, entry, hash);
            seen[state + 1] = true;
        }
        entry += 8 + size;
    }
    if (!seen[0]) {
        throw std::runtime_error("LanguageSelect: animation bank missing");
    }
}

std::array<float, 12> basis(Bytes entity) {
    std::array<float, 12> result{};
    for (std::size_t index = 0; index < result.size(); ++index) {
        result[index] = real(entity, 0x20 + index * 4);
    }
    return result;
}

std::array<float, 3> position(Bytes entity) {
    return {real(entity, 0x50), real(entity, 0x54), real(entity, 0x58)};
}
} // namespace

LanguageSelectAssets read_language_select_assets(Bytes archive) {
    const auto assets = read_asset_map(archive);
    const auto screen = find_entity(assets, "ScreenLanguageSelect",
                                    "HybridPresentationScreenBasic");
    const auto menu = find_entity(assets, "Menu_LanguageSelectMenu",
                                  "HybridPresentationMenuBasic");
    const auto camera = find_entity(assets, "CameraLanguageSelectMenu",
                                    "HybridPresentationCameraFixed");
    const std::uint32_t menu_hash = name_hash("Menu_LanguageSelectMenu");
    expect_children(screen, {&menu_hash, 1});
    const auto camera_hash = name_hash("CameraLanguageSelectMenu");
    bool camera_referenced = false;
    for (std::size_t offset = 0; offset + 4 <= screen.size(); offset += 4) {
        camera_referenced |= word(screen, offset) == camera_hash;
    }
    if (!camera_referenced) {
        throw std::runtime_error("LanguageSelect: screen camera reference missing");
    }

    LanguageSelectAssets result;
    result.font = read_font(assets);
    result.background = read_texture(assets, "tf_art_PH_bck_1");
    result.glow_lamp = read_texture(assets, "ET_glow_lamp");
    const auto glyph_map = assets.find(name_hash("LanguageSelectionGlyphs"));
    if (glyph_map == assets.end() || glyph_map->second.type != 0x6e39u) {
        throw std::runtime_error("LanguageSelect: glyph mapping missing");
    }
    constexpr std::string_view select_binding =
        "KEYBOARD_MENU_SELECT:tf_key_return";
    const auto mapping = glyph_map->second.data;
    const auto binding = std::search(mapping.begin(), mapping.end(),
        select_binding.begin(), select_binding.end());
    if (binding == mapping.end() ||
        std::size_t(mapping.end() - binding) <= select_binding.size() ||
        binding[select_binding.size()] != 0) {
        throw std::runtime_error("LanguageSelect: keyboard select glyph missing");
    }
    result.keyboard_select_glyph = "tf_key_return";
    result.keyboard_select_icon = read_texture(assets, result.keyboard_select_glyph);
    result.camera_basis = basis(camera);
    result.camera_position = position(camera);
    std::array<std::uint32_t, 16> group_hashes{};
    for (int index = 0; index < 16; ++index) {
        group_hashes[index] = name_hash("Group_" + std::to_string(index));
    }
    expect_children(menu, group_hashes);
    for (int index = 0; index < 16; ++index) {
        const auto text_name = "text_LangaugeSelectMenu" + std::to_string(index);
        const auto marker_name = "Marker_" + std::to_string(index);
        const auto group = find_entity(assets, "Group_" + std::to_string(index),
                                       "HybridPresentationGroupBasic");
        const std::array<std::uint32_t, 2> expected = {
            name_hash(text_name), name_hash(marker_name)};
        expect_children(group, expected);
        const auto text = find_entity(assets, text_name,
                                      "HybridPresentationTextButton");
        auto& item = result.texts[index];
        item.key = property_string(text, 0x74, 0x0904ec44u);
        item.font = property_string(text, 0x80, 0x05672b55u);
        if (item.key != "GUI_LANGUAGEMENU" + std::to_string(index) ||
            item.font != "LS_Ascender_Uni_211") {
            throw std::runtime_error("LanguageSelect: unexpected text key or font");
        }
        const auto height_bits = property_word(text, 0x8c, 0x0beccdb4u);
        std::memcpy(&item.line_height, &height_bits, sizeof(item.line_height));
        item.font_colour = property_word(text, 0x98, 0x0aa50092u);
        item.vertical_align = property_word(text, 0xa4, 0x0cef803eu);
        item.horizontal_align = property_word(text, 0xb0, 0x0d0a4b62u);
        item.basis = basis(text);
        item.position = position(text);
        read_button_animation(text, item);
        const auto marker = find_entity(assets, marker_name,
                                        "HybridPresentationFocusLocatorMovable");
        auto& marker_item = result.markers[index];
        marker_item.resource = property_string(marker, 0x64, 0x0174e085u);
        if (marker_item.resource != "Menu_Highlight_Left") {
            throw std::runtime_error("LanguageSelect: unexpected marker resource");
        }
        marker_item.basis = basis(marker);
        marker_item.position = position(marker);
    }
    return result;
}

LanguageSelectHighlightAssets read_language_select_highlight_assets(Bytes archive) {
    const auto assets = read_asset_map(archive);
    const auto screen = find_hybrid(assets, "ScreenMenuHighlightLeft");
    constexpr std::array<std::string_view, 5> names = {
        "Marker_ScreenMenuHighlightFollowerLeft",
        "text_HighlightGlyphLeft", "bitmap_Highlight_underlineLeft",
        "text_HighlightText", "Bitmap_HighlightBracketLeft"};
    std::array<std::uint32_t, 5> hashes{};
    LanguageSelectHighlightAssets result;
    for (std::size_t i = 0; i < names.size(); ++i) {
        hashes[i] = name_hash(names[i]);
        result.children[i] = names[i];
    }
    expect_children(screen, hashes);

    const auto follower = find_hybrid(assets, names[0]);
    const auto glyph = find_hybrid(assets, names[1]);
    const auto underline = find_hybrid(assets, names[2]);
    const auto bracket = find_hybrid(assets, names[4]);
    result.locator_name = property_string(follower, 0x64, 0x0de317f7u);
    result.glyph_markup = property_string(glyph, 0x74, 0x0904ec44u);
    result.glyph_font = property_string(glyph, 0x80, 0x05672b55u);
    result.underline_texture = property_string(underline, 0x98, 0x002c42e5u);
    result.bracket_texture = property_string(bracket, 0x98, 0x002c42e5u);
    if (result.locator_name != "Menu_Highlight_Left" ||
        result.glyph_markup != "\\[MENU_SELECT\\]" ||
        result.glyph_font != "Gomorrah" ||
        result.underline_texture != "PT_highlight_CM.tga" ||
        result.bracket_texture != "PT_menu_bracket_CM.tga") {
        throw std::runtime_error("LanguageSelect: unexpected highlight binding");
    }
    result.follower_position = position(follower);
    result.follower_basis = basis(follower);
    result.glyph_position = position(glyph);
    const auto glyph_height_bits = property_word(glyph, 0x8c, 0x0beccdb4u);
    std::memcpy(&result.glyph_line_height, &glyph_height_bits,
                sizeof(result.glyph_line_height));
    result.underline_position = position(underline);
    result.bracket_position = position(bracket);
    result.underline_basis = basis(underline);
    result.bracket_basis = basis(bracket);
    result.underline = read_texture(assets, "PT_highlight_CM");
    result.bracket = read_texture(assets, "PT_menu_bracket_CM");
    return result;
}

SasAnimationBankAsset read_sas_animation_bank(Bytes archive,
                                               std::uint32_t bank_hash) {
    const auto assets = read_asset_map(archive);
    const auto found = assets.find(bank_hash);
    if (found == assets.end() || found->second.type != 0x000684fdu) {
        throw std::runtime_error("LanguageSelect: SAS animation bank missing");
    }
    const auto data = found->second.data;
    if (data.size() < 20 || std::memcmp(data.data(), "MINA", 4) != 0) {
        throw std::runtime_error("LanguageSelect: invalid ANIM bank header");
    }
    const auto count = word(data, 16);
    if (count > (data.size() - 20) / 12) {
        throw std::runtime_error("LanguageSelect: truncated ANIM channel table");
    }
    SasAnimationBankAsset result;
    result.hash = bank_hash;
    result.channels.reserve(count);
    for (std::uint32_t index = 0; index < count; ++index) {
        const auto entry = 20u + index * 12u;
        const auto name_offset = word(data, entry);
        const auto hash = word(data, entry + 4);
        const auto curve_offset = word(data, entry + 8);
        if (name_offset >= data.size() ||
            curve_offset > data.size() || data.size() - curve_offset < 60) {
            throw std::runtime_error("LanguageSelect: invalid ANIM channel offset");
        }
        const auto* name_begin = reinterpret_cast<const char*>(
            data.data() + name_offset);
        const auto* name_end = static_cast<const char*>(
            std::memchr(name_begin, 0, data.size() - name_offset));
        if (!name_end) {
            throw std::runtime_error("LanguageSelect: unterminated ANIM channel");
        }
        SasAnimationChannelAsset channel;
        channel.name.assign(name_begin, name_end);
        channel.hash = hash;
        if (channel_hash(channel.name) != hash) {
            throw std::runtime_error("LanguageSelect: ANIM channel hash mismatch");
        }
        std::copy_n(data.begin() + curve_offset + 4, 4,
                    channel.flags.begin());
        const auto key_count = word(data, curve_offset);
        if (key_count > (data.size() - curve_offset - 60) / 24) {
            throw std::runtime_error("LanguageSelect: truncated ANIM curve");
        }
        channel.keys.reserve(key_count);
        for (std::uint32_t key = 0; key < key_count; ++key) {
            const auto offset = curve_offset + 60u + key * 24u;
            SasAnimationKeyAsset record;
            record.time = real(data, offset);
            record.value = real(data, offset + 4);
            for (std::size_t component = 0; component < 4; ++component) {
                record.tangents[component] = real(data, offset + 8 + component * 4);
            }
            channel.keys.push_back(record);
        }
        result.channels.push_back(std::move(channel));
    }
    return result;
}

} // namespace hp6::presentation
