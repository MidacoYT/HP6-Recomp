#include "presentation/PresentationStringLayout.h"

#include <algorithm>
#include <stdexcept>

namespace hp6::presentation {

ShapedPresentationString shape_plain_presentation_string(
    std::wstring_view text, const LanguageSelectFontAsset& font) {
    ShapedPresentationString result;
    const auto fallback = font.glyphs.find(' ');
    if (fallback == font.glyphs.end()) {
        throw std::invalid_argument("PresentationString: space glyph missing");
    }
    std::size_t first = 0;
    float cursor = 0.0f;
    for (const wchar_t character : text) {
        if (character == L'\n') {
            result.lines.push_back({first, result.glyphs.size()});
            first = result.glyphs.size();
            cursor = 0.0f;
            continue;
        }
        const bool tab = character == L'\t';
        const auto codepoint = tab ? std::uint16_t(' ') :
            static_cast<std::uint16_t>(character);
        const auto found = font.glyphs.find(codepoint);
        const auto& source = found == font.glyphs.end() ?
            fallback->second : found->second;
        const float advance = float(source.advance) * (tab ? 3.0f : 1.0f);
        PresentationGlyph glyph;
        glyph.flags = (character == L' ' || tab) ? 0x101u :
                      character == L'\u00a0' ? 0x100u : 0u;
        glyph.x_left = cursor;
        glyph.x_right = cursor + advance;
        glyph.y_top = -float(source.top_offset);
        glyph.y_bottom = glyph.y_top - float(source.height);
        glyph.texture[0] = source.atlas_u;
        glyph.texture[1] = std::uint32_t(source.atlas_u) + source.width + 1;
        glyph.texture[2] = source.atlas_v;
        glyph.texture[3] = std::uint32_t(source.atlas_v) + source.height + 1;
        result.glyphs.push_back(glyph);
        cursor += advance;
    }
    result.lines.push_back({first, result.glyphs.size()});
    return result;
}

void layout_presentation_string_glyphs(
    std::span<PresentationGlyph> glyphs,
    std::span<const PresentationGlyphLine> lines,
    const PresentationStringDescriptor& descriptor,
    float font_height) {
    if (font_height == 0.0f) {
        throw std::invalid_argument("PresentationString: zero font height");
    }
    const float scale = descriptor.line_height / font_height;
    for (auto& glyph : glyphs) {
        glyph.x_left *= scale;
        glyph.x_right *= scale;
    }
    float line_offset = 0.0f;
    for (const auto& line : lines) {
        if (line.first > line.end || line.end > glyphs.size()) {
            throw std::invalid_argument("PresentationString: invalid line range");
        }
        for (std::size_t index = line.first; index < line.end; ++index) {
            glyphs[index].y_top = glyphs[index].y_top * scale + line_offset;
            glyphs[index].y_bottom = glyphs[index].y_bottom * scale + line_offset;
        }
        line_offset -= descriptor.line_spacing * descriptor.line_height;
    }
    for (std::size_t line_index = 0; line_index < lines.size(); ++line_index) {
        const auto& line = lines[line_index];
        if (line.first == line.end) continue;
        const auto left = glyphs[line.first].x_left;
        const auto right = glyphs[line.end - 1].x_right;
        float shift = descriptor.rectangle[0] - left;
        float extra_space = 0.0f;
        switch (descriptor.horizontal_align) {
        case 1:
            shift = (descriptor.rectangle[0] + descriptor.rectangle[1] -
                     left - right) * 0.5f;
            break;
        case 2:
            shift = descriptor.rectangle[1] - right;
            break;
        case 3:
            if (line_index + 1 < lines.size()) {
                std::size_t spaces = 0;
                for (std::size_t index = line.first; index < line.end; ++index) {
                    spaces += (glyphs[index].flags & 0xffu) == 1;
                }
                if (spaces != 0) {
                    extra_space = (descriptor.rectangle[1] - right - shift) /
                                  static_cast<float>(spaces);
                }
            }
            break;
        default:
            break;
        }
        for (std::size_t index = line.first; index < line.end; ++index) {
            glyphs[index].x_left += shift;
            glyphs[index].x_right += shift;
            if ((glyphs[index].flags & 0xffu) == 1) shift += extra_space;
        }
    }
    // 0x005fa800: the serialized value 1 centres the union of glyph bounds
    // about the string component's origin. Values 0 and 2 pin the top or
    // bottom to the descriptor's rectangle respectively.
    if (!glyphs.empty()) {
        float top = glyphs.front().y_top;
        float bottom = glyphs.front().y_bottom;
        for (const auto& glyph : glyphs) {
            top = std::max(top, glyph.y_top);
            bottom = std::min(bottom, glyph.y_bottom);
        }
        float shift = descriptor.rectangle[3] - top;
        if (descriptor.vertical_align == 1) {
            shift = -(top + bottom) * 0.5f;
        } else if (descriptor.vertical_align == 2) {
            shift = descriptor.rectangle[2] - bottom;
        }
        for (auto& glyph : glyphs) {
            glyph.y_top += shift;
            glyph.y_bottom += shift;
        }
    }
}

} // namespace hp6::presentation
