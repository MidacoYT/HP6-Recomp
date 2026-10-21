#pragma once

#include <array>
#include <cstdint>
#include <string>

#include "presentation/BitmapRenderQuad.h"

namespace hp6::presentation {

// Values assembled by PluginPresentationString's render slot 0x0047a9e0
// before it calls the separate text layout path at 0x005f9bc0.
struct PresentationStringDescriptor {
    std::wstring text;
    std::string font;
    std::uint32_t colour = 0;
    std::array<float, 16> matrix{};
    std::array<float, 4> rectangle{};
    float line_height = 0.0f;
    float line_spacing = 0.0f;
    std::uint32_t horizontal_align = 0;
    std::uint32_t vertical_align = 0;
    std::uint32_t shadow_offset = 0;
    std::uint32_t shadow_colour = 0;
    std::uint8_t option_bits = 0;
    BitmapCoordinateSpace space{};
};

// The caller resolves the text through the presentation string service.
// This function reconstructs the deterministic field transfer in the
// DAT_00fcae50+8 == 2 branch, including the visibility gate. The later
// glyph layout/submission call (0x005f9bc0) is not included here.
bool build_presentation_string_descriptor(const void* string_component,
                                          const float* parent_matrix,
                                          const wchar_t* resolved_text,
                                          BitmapCoordinateSpace space,
                                          PresentationStringDescriptor& out);

} // namespace hp6::presentation
