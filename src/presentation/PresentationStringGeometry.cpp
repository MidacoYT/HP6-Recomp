#include "presentation/PresentationStringGeometry.h"

#include <limits>
#include <stdexcept>

namespace hp6::presentation {
namespace {
std::uint16_t atlas_coordinate(std::uint32_t coordinate, std::uint16_t size) {
    // 0x005fabd0: (texel * 0x1000 - 0x800) / atlas dimension,
    // truncated toward zero, then retained as a 16-bit vertex attribute.
    return static_cast<std::uint16_t>(
        (std::int64_t(coordinate) * 0x1000 - 0x800) / size);
}
} // namespace

PresentationStringGeometry build_presentation_string_geometry(
    std::span<const PresentationGlyph> glyphs,
    const LanguageSelectFontAsset& font) {
    if (font.atlas_width == 0 || font.atlas_height == 0) {
        throw std::invalid_argument("PresentationString: atlas dimensions missing");
    }
    PresentationStringGeometry result;
    for (const auto& glyph : glyphs) {
        if (glyph.tag != 0 || (glyph.flags & 0xff00u) != 0) continue;
        if (result.vertices.size() >
            std::size_t(std::numeric_limits<std::uint16_t>::max()) - 3) {
            throw std::overflow_error("PresentationString: too many vertices");
        }
        const auto base = static_cast<std::uint16_t>(result.vertices.size());
        const auto u_left = atlas_coordinate(glyph.texture[0], font.atlas_width);
        const auto u_right = atlas_coordinate(glyph.texture[1], font.atlas_width);
        const auto v_top = atlas_coordinate(glyph.texture[2], font.atlas_height);
        const auto v_bottom = atlas_coordinate(glyph.texture[3], font.atlas_height);
        result.vertices.push_back({glyph.x_left, glyph.y_top, 0.0f, u_left, v_top});
        result.vertices.push_back({glyph.x_right, glyph.y_top, 0.0f, u_right, v_top});
        result.vertices.push_back({glyph.x_left, glyph.y_bottom, 0.0f, u_left, v_bottom});
        result.vertices.push_back({glyph.x_right, glyph.y_bottom, 0.0f, u_right, v_bottom});
        for (std::uint16_t index :
             {std::uint16_t{0}, std::uint16_t{1}, std::uint16_t{2},
              std::uint16_t{1}, std::uint16_t{3}, std::uint16_t{2}}) {
            result.indices.push_back(base + index);
        }
    }
    return result;
}

} // namespace hp6::presentation
