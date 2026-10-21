#include "presentation/LanguageSelectAssets.h"
#include "presentation/LanguageSelectNames.h"
#include "presentation/PresentationFocusRegistry.h"
#include "presentation/PresentationStringLayout.h"
#include "presentation/PresentationStringGeometry.h"
#include "presentation/ShpiDxtDecode.h"
#include "presentation/SasCurveEvaluator.h"
#include "presentation/SasColorAnimation.h"
#include "math/Transform.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <string_view>
#include <vector>
#ifdef _MSC_VER
#include <crtdbg.h>
#endif

int main(int argc, char** argv) {
#ifdef _MSC_VER
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);
#endif
    assert(argc == 3);
    std::ifstream file(argv[1], std::ios::binary);
    assert(file);
    const std::vector<std::uint8_t> archive(
        std::istreambuf_iterator<char>{file}, std::istreambuf_iterator<char>{});
    const auto assets = hp6::presentation::read_language_select_assets(archive);
    for (int index = 0; index < 16; ++index) {
        const auto& item = assets.texts[index];
        assert(item.key == "GUI_LANGUAGEMENU" + std::to_string(index));
        assert(item.font == "LS_Ascender_Uni_211");
        assert(std::fabs(item.line_height - 0.065f) < 0.00001f);
        assert(item.font_colour == 0xd6d8b3ffu);
        assert(item.vertical_align == 1u);
        assert(item.horizontal_align == 5u);
        assert(item.position[0] > 35.0f && item.position[0] < 36.0f);
        assert(item.animation_bank_hash == 0x08eaad23u);
        assert(item.animation_clips[0] == "fadein");
        assert(item.animation_clips[5] == "Rotate");
        for (std::size_t state = 0; state < item.animation_clips.size(); ++state) {
            if (state != 0 && state != 5) {
                assert(item.animation_clips[state].empty());
            }
        }
        const auto& marker = assets.markers[index];
        assert(marker.resource == "Menu_Highlight_Left");
        assert(marker.basis[0] > 0.0f);
    }
    assert(assets.texts[0].position[0] < assets.texts[8].position[0]);
    assert(assets.texts[0].position[1] > assets.texts[7].position[1]);
    assert(assets.texts[8].position[1] > assets.texts[15].position[1]);
    assert(assets.markers[0].position[1] > assets.markers[7].position[1]);
    assert(assets.markers[8].position[1] > assets.markers[15].position[1]);
    assert(std::fabs(assets.camera_position[0] - 36.31377f) < 0.0001f);
    assert(std::fabs(assets.camera_position[1] + 144.20711f) < 0.0001f);
    assert(assets.font.metric_height == 29);
    assert(assets.font.atlas_width == 256 && assets.font.atlas_height == 256);
    assert(assets.font.glyphs.size() == 128);
    assert(assets.font.alpha.size() == 256u * 256u);
    assert(assets.background.format == 0x60);
    assert(assets.background.width == 512 &&
           assets.background.height == 512);
    assert(assets.background.mip0_blocks.size() == 512u * 512u / 2u);
    assert(assets.background.mip_levels == 10);
    assert(assets.background.mip_chain_blocks.size() == 174776);
    assert(assets.glow_lamp.width == 128 && assets.glow_lamp.height == 128);
    assert(assets.glow_lamp.format >= 0x60 && assets.glow_lamp.format <= 0x62);
    assert(hp6::presentation::decode_shpi_mip0_rgba(
               assets.glow_lamp).size() == 128u * 128u * 4u);
    assert(assets.keyboard_select_glyph == "tf_key_return");
    assert(assets.keyboard_select_icon.width == 128);
    assert(assets.keyboard_select_icon.height == 128);
    assert(std::string_view(hp6::presentation::kInstalledLanguageNames[1].locale)
           == "fr-fr");
    assert(std::string_view(hp6::presentation::kInstalledLanguageNames[8].locale)
           == "ru-ru");
    const auto background_pixels =
        hp6::presentation::decode_shpi_mip0_rgba(assets.background);
    assert(background_pixels.size() == 512u * 512u * 4u);
    const auto background_middle = (256u * 512u + 256u) * 4u;
    assert(std::abs(int(background_pixels[background_middle]) - 132) <= 2);
    assert(std::abs(int(background_pixels[background_middle + 1]) - 117) <= 2);
    assert(std::abs(int(background_pixels[background_middle + 2]) - 99) <= 2);
    assert(background_pixels[background_middle + 3] == 255);
    const auto capital_a = assets.font.glyphs.at('A');
    assert(capital_a.width == 18 && capital_a.height == 18);
    assert(capital_a.advance == 18);
    assert(capital_a.top_offset == 6);
    auto shaped = hp6::presentation::shape_plain_presentation_string(
        L"A A", assets.font);
    assert(shaped.glyphs.size() == 3 && shaped.lines.size() == 1);
    assert(shaped.glyphs[0].texture[0] == capital_a.atlas_u);
    assert(shaped.glyphs[1].flags == 0x101u);
    assert(shaped.glyphs[2].x_left == 27.0f);
    hp6::presentation::PresentationStringDescriptor descriptor;
    descriptor.line_height = assets.texts[0].line_height;
    descriptor.rectangle = {0.0f, 1.0f, 0.0f, 1.0f};
    hp6::presentation::layout_presentation_string_glyphs(
        shaped.glyphs, shaped.lines, descriptor,
        static_cast<float>(assets.font.metric_height));
    assert(shaped.glyphs[0].x_right > 0.04f &&
           shaped.glyphs[0].x_right < 0.041f);
    const auto geometry = hp6::presentation::build_presentation_string_geometry(
        shaped.glyphs, assets.font);
    assert(geometry.vertices.size() == 8 && geometry.indices.size() == 12);
    assert(geometry.indices[0] == 0 && geometry.indices[5] == 2);
    assert(geometry.indices[6] == 4 && geometry.indices[11] == 6);
    assert(geometry.vertices[0].u == 1416);
    assert(geometry.vertices[1].u == 1720);
    assert(geometry.vertices[0].v == 808);
    std::ifstream game_file(argv[2], std::ios::binary);
    assert(game_file);
    const std::vector<std::uint8_t> game_archive(
        std::istreambuf_iterator<char>{game_file},
        std::istreambuf_iterator<char>{});
    const auto animation_bank = hp6::presentation::read_sas_animation_bank(
        game_archive, assets.texts[0].animation_bank_hash);
    assert(animation_bank.hash == 0x08eaad23u);
    assert(animation_bank.channels.size() == 66);
    const auto rotate_colour = std::find_if(animation_bank.channels.begin(),
        animation_bank.channels.end(), [](const auto& channel) {
            return channel.name == "Rotate.R";
        });
    assert(rotate_colour != animation_bank.channels.end());
    assert(rotate_colour->keys.size() == 3);
    assert(rotate_colour->keys[0].time == 0.0f);
    assert(std::fabs(rotate_colour->keys[0].value - 0.8f) < 0.00001f);
    assert(rotate_colour->keys[2].time == 0.75f);
    const auto sample = [&](float time) {
        return hp6::presentation::sample_sas_nonweighted_curve(
            *rotate_colour, time);
    };
    assert(std::fabs(sample(0.0f) - 0.8f) < 0.00001f);
    assert(std::fabs(sample(0.1875f) - 0.8f) < 0.00001f);
    assert(std::fabs(sample(0.5625f) - 0.875f) < 0.0001f);
    assert(std::fabs(sample(0.75f) - 1.0f) < 0.00001f);
    assert(std::fabs(sample(1.3125f) - 0.875f) < 0.0001f);
    assert(std::fabs(sample(-0.1875f) - 0.875f) < 0.0001f);
    const auto scale = std::find_if(animation_bank.channels.begin(),
        animation_bank.channels.end(), [](const auto& channel) {
            return channel.name == "Rotate.scaleX";
        });
    assert(scale != animation_bank.channels.end());
    assert(scale->flags[1] == 1);
    assert(hp6::presentation::sample_sas_nonweighted_curve(*scale,
                                                            0.5625f) == 1.0f);
    const auto rotate_channels = std::count_if(animation_bank.channels.begin(),
        animation_bank.channels.end(), [](const auto& channel) {
            return channel.name.starts_with("Rotate.");
        });
    assert(rotate_channels == 13);
    const auto colour_channels = hp6::presentation::find_sas_color_channels(
        animation_bank, assets.texts[0].animation_clips[5]);
    assert(colour_channels.rgba[0] && colour_channels.rgba[1] &&
           colour_channels.rgba[2] && !colour_channels.rgba[3]);
    constexpr std::uint32_t authored_rgba = 0xffb3d8d6u;
    assert(hp6::presentation::sample_sas_color_target(
        colour_channels, authored_rgba, 0.0f) == 0xff99c2ccu);
    assert(hp6::presentation::sample_sas_color_target(
        colour_channels, authored_rgba, 0.75f) == 0xffffffffu);
    hp6::presentation::SasColorSmoother smoother(authored_rgba);
    const float colour_error = smoother.update(0xffffffffu, 100);
    assert(colour_error > 0.01f);
    assert(smoother.current() == 0xffc2e0deu);
    assert(smoother.update(0xffffffffu, 100) > 0.01f);
    assert(smoother.current() == 0xffc2e0deu);
    smoother.update(0xffffffffu, 101);
    assert((smoother.current() & 0xffu) > 0xdeu);
    assert(std::none_of(animation_bank.channels.begin(),
        animation_bank.channels.end(), [](const auto& channel) {
            return channel.name.starts_with("fadein.");
        }));
    const auto highlight =
        hp6::presentation::read_language_select_highlight_assets(game_archive);
    assert(highlight.children[0] ==
           "Marker_ScreenMenuHighlightFollowerLeft");
    assert(highlight.children[4] == "Bitmap_HighlightBracketLeft");
    assert(highlight.locator_name == "Menu_Highlight_Left");
    assert(highlight.glyph_markup == "\\[MENU_SELECT\\]");
    assert(highlight.glyph_font == "Gomorrah");
    assert(highlight.underline_texture == "PT_highlight_CM.tga");
    assert(highlight.bracket_texture == "PT_menu_bracket_CM.tga");
    assert(highlight.underline_basis[0] > 0.1f);
    assert(highlight.bracket_basis[5] > 0.1f);
    assert(std::fabs(highlight.glyph_line_height - 0.05f) < 0.00001f);
    assert(highlight.follower_basis[0] < -0.25f);
    assert(highlight.underline.format == 0x62);
    assert(highlight.underline.width == 128 &&
           highlight.underline.height == 64);
    assert(highlight.underline.mip0_blocks.size() == 8192);
    assert(highlight.underline.mip_chain_blocks.size() == 8192);
    const auto underline_pixels =
        hp6::presentation::decode_shpi_mip0_rgba(highlight.underline);
    const auto underline_middle = (32u * 128u + 64u) * 4u;
    assert(std::abs(int(underline_pixels[underline_middle]) - 150) <= 2);
    assert(std::abs(int(underline_pixels[underline_middle + 1]) - 91) <= 2);
    assert(underline_pixels[underline_middle + 3] == 236);
    assert(highlight.bracket.format == 0x62);
    assert(highlight.bracket.width == 32 &&
           highlight.bracket.height == 32);
    assert(highlight.bracket.mip_chain_blocks.size() == 1024);
    const auto bracket_pixels =
        hp6::presentation::decode_shpi_mip0_rgba(highlight.bracket);
    const auto bracket_middle = (16u * 32u + 16u) * 4u;
    assert(bracket_pixels[bracket_middle] == 255);
    assert(bracket_pixels[bracket_middle + 3] == 255);
    hp6::presentation::PresentationFocusRegistry registry;
    const hp6::presentation::PresentationMatrix identity{
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    auto follower = hp6::presentation::compose_presentation_focus_follower(
        registry, highlight.locator_name, identity);
    assert(follower == identity);
    hp6::presentation::PresentationMatrix marker{};
    std::copy(assets.markers[0].basis.begin(),
              assets.markers[0].basis.end(), marker.begin());
    std::copy(assets.markers[0].position.begin(),
              assets.markers[0].position.end(), marker.begin() + 12);
    marker[15] = 1.0f;
    registry.publish(assets.markers[0].resource, marker);
    follower = hp6::presentation::compose_presentation_focus_follower(
        registry, highlight.locator_name, identity);
    assert(follower == marker);
    hp6::presentation::PresentationMatrix authored{};
    std::copy(highlight.follower_basis.begin(),
              highlight.follower_basis.end(), authored.begin());
    std::copy(highlight.follower_position.begin(),
              highlight.follower_position.end(), authored.begin() + 12);
    authored[15] = 1.0f;
    const auto inverse =
        hp6::presentation::make_presentation_focus_follower_local(authored);
    hp6::presentation::PresentationMatrix round_trip{};
    hp6::math::multiply_matrix4x4(round_trip.data(), authored.data(),
                                  inverse.data());
    for (int row = 0; row < 4; ++row) {
        for (int column = 0; column < 4; ++column) {
            const float expected = row == column ? 1.0f : 0.0f;
            assert(std::fabs(round_trip[row * 4 + column] - expected) < 0.0001f);
        }
    }
    const auto source_to_marker =
        hp6::presentation::compose_presentation_focus_follower(
            registry, highlight.locator_name, inverse);
    hp6::presentation::PresentationMatrix glyph_transform{
        1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0,
        highlight.glyph_position[0], highlight.glyph_position[1],
        highlight.glyph_position[2], 1};
    hp6::presentation::PresentationMatrix placed_glyph{};
    hp6::math::multiply_matrix4x4(placed_glyph.data(),
                                  glyph_transform.data(), source_to_marker.data());
    assert(std::fabs(placed_glyph[12] - marker[12]) < 0.01f);
    assert(std::fabs(placed_glyph[13] - marker[13]) < 0.01f);
    assert(std::fabs(placed_glyph[14] - marker[14]) < 0.01f);
    auto local = identity;
    local[12] = 2.0f;
    local[13] = -3.0f;
    follower = hp6::presentation::compose_presentation_focus_follower(
        registry, highlight.locator_name, local);
    assert(std::fabs(follower[12] - (2.0f * marker[0] -
           3.0f * marker[4] + marker[12])) < 0.0001f);
    assert(std::fabs(follower[13] - (2.0f * marker[1] -
           3.0f * marker[5] + marker[13])) < 0.0001f);
    registry.set_alias("language_focus", highlight.locator_name);
    follower = hp6::presentation::compose_presentation_focus_follower(
        registry, "language_focus", identity);
    assert(follower == marker);
    registry.remove(highlight.locator_name);
    follower = hp6::presentation::compose_presentation_focus_follower(
        registry, highlight.locator_name, identity);
    assert(follower == identity);
    {
        hp6::presentation::PresentationFocusLocatorMovable first(
            registry, assets.markers[0].resource, marker);
        hp6::presentation::PresentationFocusPhase first_phase(first);
        assert(first_phase.state() == 3);
        assert(first_phase.request_blur());
        assert(first_phase.state() == 3);
        assert(first_phase.request_focus());
        assert(first_phase.state() == 4);
        assert(!first_phase.tick(false));
        assert(first_phase.state() == 4);
        assert(!registry.lookup(highlight.locator_name, follower));
        assert(first_phase.tick(true));
        assert(first_phase.state() == 6);
        assert(registry.lookup(highlight.locator_name, follower));
        assert(follower == marker);

        auto second_marker = marker;
        second_marker[12] += 3.0f;
        hp6::presentation::PresentationFocusLocatorMovable second(
            registry, assets.markers[1].resource, second_marker);
        hp6::presentation::PresentationFocusPhase second_phase(second);
        assert(first_phase.request_blur());
        assert(first_phase.state() == 5);
        assert(second_phase.request_focus());
        assert(second_phase.state() == 4);
        assert(!second_phase.tick(false));
        assert(registry.lookup(highlight.locator_name, follower));
        assert(follower == marker);
        assert(first_phase.tick(true));
        assert(second_phase.tick(true));
        assert(second_phase.state() == 6);
        assert(registry.lookup(highlight.locator_name, follower));
        assert(follower == second_marker);
        assert(second_phase.request_focus());
        assert(!second_phase.tick(true));
    }
    registry.remove(highlight.locator_name);
    {
        hp6::presentation::PresentationFocusLocatorMovable locator(
            registry, assets.markers[0].resource, marker);
        follower = hp6::presentation::compose_presentation_focus_follower(
            registry, highlight.locator_name, identity);
        assert(follower == identity);
        hp6::presentation::PresentationMatrix next_marker{};
        std::copy(assets.markers[1].basis.begin(),
                  assets.markers[1].basis.end(), next_marker.begin());
        std::copy(assets.markers[1].position.begin(),
                  assets.markers[1].position.end(), next_marker.begin() + 12);
        next_marker[15] = 1.0f;
        locator.transition_to(4);
        locator.update(next_marker);
        follower = hp6::presentation::compose_presentation_focus_follower(
            registry, highlight.locator_name, identity);
        assert(follower == identity);
        registry.publish("fallback_marker", marker);
        registry.set_alias(highlight.locator_name, "fallback_marker");
        locator.transition_to(6);
        follower = hp6::presentation::compose_presentation_focus_follower(
            registry, highlight.locator_name, identity);
        assert(follower == next_marker);
        locator.update(marker);
        locator.transition_to(3);
        locator.update(next_marker);
        hp6::presentation::PresentationFocusLocatorMovable next_locator(
            registry, assets.markers[1].resource, next_marker);
        next_locator.transition_to(6);
        follower = hp6::presentation::compose_presentation_focus_follower(
            registry, highlight.locator_name, identity);
        assert(follower == next_marker);
    }
    follower = hp6::presentation::compose_presentation_focus_follower(
        registry, highlight.locator_name, identity);
    assert(follower[12] == assets.markers[1].position[0]);
    registry.remove(highlight.locator_name);
    follower = hp6::presentation::compose_presentation_focus_follower(
        registry, highlight.locator_name, identity);
    assert(follower == identity);
    const auto locale_dir = std::filesystem::path(argv[2]).parent_path() /
                            "game/localised";
    for (const auto& language : hp6::presentation::kInstalledLanguageNames) {
        assert(std::filesystem::exists(locale_dir /
                   (std::string(language.locale) + ".str")));
        for (const wchar_t character : std::wstring_view(language.display)) {
            assert(assets.font.glyphs.contains(
                static_cast<std::uint16_t>(character)));
        }
    }
}
