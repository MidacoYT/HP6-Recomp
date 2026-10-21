#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <vector>

#include "presentation/LanguageSelectAssets.h"
#include "presentation/PresentationStringRender.h"

namespace hp6::presentation {

// The 0x28-byte glyph element emitted by 0x005f9e60. The four trailing
// dwords hold glyph texture coordinates and are untouched by this stage.
struct PresentationGlyph {
    std::uint32_t tag = 0;
    std::uint32_t flags = 0; // low byte 1: expandable space
    float x_left = 0.0f;
    float x_right = 0.0f;
    float y_top = 0.0f;
    float y_bottom = 0.0f;
    std::uint32_t texture[4]{};
};
static_assert(sizeof(PresentationGlyph) == 0x28);

struct PresentationGlyphLine {
    std::size_t first = 0;
    std::size_t end = 0;
};

struct ShapedPresentationString {
    std::vector<PresentationGlyph> glyphs;
    std::vector<PresentationGlyphLine> lines;
};

// Plain glyph branch of 0x005f9e60. Unknown codepoints use the font's
// space glyph, as in the original; markup tags are not handled here.
ShapedPresentationString shape_plain_presentation_string(
    std::wstring_view text, const LanguageSelectFontAsset& font);

// The x scaling in 0x005f9bc0, followed by 0x005fa5e0 and 0x005fa680.
// `font_height` is the font metric at font->glyph_table->+0x24.
void layout_presentation_string_glyphs(
    std::span<PresentationGlyph> glyphs,
    std::span<const PresentationGlyphLine> lines,
    const PresentationStringDescriptor& descriptor,
    float font_height);

} // namespace hp6::presentation
