#pragma once

#include <array>
#include <cstdint>
#include <map>
#include <span>
#include <string>
#include <vector>

namespace hp6::presentation {

struct LanguageSelectTextAsset {
    std::string key;
    std::string font;
    float line_height = 0.0f;
    std::uint32_t font_colour = 0;
    std::uint32_t vertical_align = 0;
    std::uint32_t horizontal_align = 0;
    std::array<float, 12> basis{};
    std::array<float, 3> position{};
    // PluginPresentationButtonTextAnimation (property 95): state 1..9 maps
    // to clips[0]..clips[8]. Empty strings are the native zero defaults.
    std::uint32_t animation_bank_hash = 0;
    std::array<std::string, 9> animation_clips{};
};

struct LanguageSelectMarkerAsset {
    std::string resource;
    std::array<float, 12> basis{};
    std::array<float, 3> position{};
};

struct LanguageSelectFontGlyph {
    std::uint8_t width = 0;
    std::uint8_t height = 0;
    std::int8_t top_offset = 0;
    std::uint16_t atlas_u = 0;
    std::uint16_t atlas_v = 0;
    std::uint16_t advance = 0;
};

struct LanguageSelectFontAsset {
    std::uint32_t metric_height = 0;
    std::uint16_t atlas_width = 0;
    std::uint16_t atlas_height = 0;
    std::map<std::uint16_t, LanguageSelectFontGlyph> glyphs;
    std::vector<std::uint8_t> alpha;
};

struct LanguageSelectTextureAsset {
    std::uint16_t width = 0;
    std::uint16_t height = 0;
    std::uint8_t format = 0; // SHPI 0x60 DXT1, 0x61 DXT3, 0x62 DXT5
    std::uint8_t mip_levels = 0;
    std::vector<std::uint8_t> mip0_blocks;
    std::vector<std::uint8_t> mip_chain_blocks;
};

struct LanguageSelectAssets {
    std::array<float, 12> camera_basis{};
    std::array<float, 3> camera_position{};
    std::array<LanguageSelectTextAsset, 16> texts{};
    std::array<LanguageSelectMarkerAsset, 16> markers{};
    LanguageSelectFontAsset font;
    LanguageSelectTextureAsset background;
    LanguageSelectTextureAsset glow_lamp;
    std::string keyboard_select_glyph;
    LanguageSelectTextureAsset keyboard_select_icon;
};

struct LanguageSelectHighlightAssets {
    std::array<std::string, 5> children;
    std::string locator_name;
    std::string glyph_markup;
    std::string glyph_font;
    std::string underline_texture;
    std::string bracket_texture;
    std::array<float, 3> follower_position{};
    std::array<float, 12> follower_basis{};
    std::array<float, 3> glyph_position{};
    float glyph_line_height = 0.0f;
    std::array<float, 3> underline_position{};
    std::array<float, 3> bracket_position{};
    std::array<float, 12> underline_basis{};
    std::array<float, 12> bracket_basis{};
    LanguageSelectTextureAsset underline;
    LanguageSelectTextureAsset bracket;
};

struct SasAnimationKeyAsset {
    float time = 0.0f;
    float value = 0.0f;
    std::array<float, 4> tangents{};
};

struct SasAnimationChannelAsset {
    std::string name;
    std::uint32_t hash = 0;
    std::array<std::uint8_t, 4> flags{};
    std::vector<SasAnimationKeyAsset> keys;
};

struct SasAnimationBankAsset {
    std::uint32_t hash = 0;
    std::vector<SasAnimationChannelAsset> channels;
};

// Read the original LanguageSelect STRM/AGRP/ASET container and validate its
// Screen -> Menu -> Group_i -> Text_i/Marker_i hierarchy. The returned data
// is owned by the caller; no pointers into the archive survive this call.
LanguageSelectAssets read_language_select_assets(
    std::span<const std::uint8_t> archive);

// The five children of ScreenMenuHighlightLeft are stored in game.str, not
// languageselect.str. The follower binds to each chapter's movable locator.
LanguageSelectHighlightAssets read_language_select_highlight_assets(
    std::span<const std::uint8_t> game_archive);

// Read the serialized ANIM bank referenced by the language TextButtons.
// This exposes original channels and keys; it does not evaluate SAS curves.
SasAnimationBankAsset read_sas_animation_bank(
    std::span<const std::uint8_t> archive, std::uint32_t bank_hash);

} // namespace hp6::presentation
