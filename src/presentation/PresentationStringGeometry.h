#pragma once

#include <cstdint>
#include <span>
#include <vector>

#include "presentation/PresentationStringLayout.h"

namespace hp6::presentation {

// Four source vertices per ordinary glyph in 0x005fabd0. Their texture
// coordinates are the 12-bit fixed values consumed by vertex format 0x12.
struct PresentationStringVertex {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    std::uint16_t u = 0;
    std::uint16_t v = 0;
};

struct PresentationStringGeometry {
    std::vector<PresentationStringVertex> vertices;
    std::vector<std::uint16_t> indices;
};

// CPU geometry immediately before FUN_00641600's batch write. Plain spaces
// and nonbreaking spaces have no visible quad. Markup/tag glyphs belong to
// the separate 0x005fafb0 pass and are intentionally omitted here.
PresentationStringGeometry build_presentation_string_geometry(
    std::span<const PresentationGlyph> glyphs,
    const LanguageSelectFontAsset& font);

} // namespace hp6::presentation
