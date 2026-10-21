#include "presentation/PresentationStringLayout.h"

#include <array>
#include <cassert>
#include <cmath>

int main() {
    using namespace hp6::presentation;
    PresentationStringDescriptor descriptor;
    descriptor.line_height = 10.0f;
    descriptor.line_spacing = 0.25f;
    descriptor.rectangle = {0.0f, 100.0f, 0.0f, 100.0f};
    descriptor.horizontal_align = 3;
    std::array<PresentationGlyph, 4> glyphs{};
    glyphs[0].x_left = 0.0f;  glyphs[0].x_right = 10.0f;
    glyphs[1].flags = 1; glyphs[1].x_left = 10.0f;
    glyphs[1].x_right = 15.0f;
    glyphs[2].x_left = 15.0f; glyphs[2].x_right = 30.0f;
    glyphs[3].x_left = 0.0f; glyphs[3].x_right = 20.0f;
    for (auto& glyph : glyphs) {
        glyph.y_top = 0.0f;
        glyph.y_bottom = -10.0f;
    }
    const std::array lines{PresentationGlyphLine{0, 3},
                           PresentationGlyphLine{3, 4}};
    layout_presentation_string_glyphs(glyphs, lines, descriptor, 20.0f);
    assert(glyphs[0].x_left == 0.0f);
    assert(glyphs[1].x_right == 7.5f);
    assert(glyphs[2].x_left == 92.5f);
    assert(glyphs[2].x_right == 100.0f);
    assert(glyphs[3].x_right == 10.0f); // final line is left aligned
    assert(glyphs[0].y_bottom == 95.0f);
    assert(glyphs[3].y_bottom == 92.5f);

    descriptor.horizontal_align = 1;
    std::array<PresentationGlyph, 1> centered{};
    centered[0].x_left = 0.0f;
    centered[0].x_right = 20.0f;
    const std::array one_line{PresentationGlyphLine{0, 1}};
    layout_presentation_string_glyphs(centered, one_line, descriptor, 10.0f);
    assert(centered[0].x_left == 40.0f && centered[0].x_right == 60.0f);
    assert(centered[0].y_top == 100.0f);

    descriptor.vertical_align = 1;
    std::array<PresentationGlyph, 1> middle{};
    middle[0].y_top = -3.0f;
    middle[0].y_bottom = -9.0f;
    layout_presentation_string_glyphs(middle, one_line, descriptor, 10.0f);
    assert(middle[0].y_top == 3.0f && middle[0].y_bottom == -3.0f);

    descriptor.vertical_align = 2;
    std::array<PresentationGlyph, 1> bottom{};
    bottom[0].y_top = -3.0f;
    bottom[0].y_bottom = -9.0f;
    layout_presentation_string_glyphs(bottom, one_line, descriptor, 10.0f);
    assert(bottom[0].y_bottom == 0.0f);
}
