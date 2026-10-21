#include "presentation/PresentationStringRender.h"

#include <cstring>

namespace hp6::presentation {
namespace {
template <typename T> T field(const void* object, std::size_t offset) {
    T result{};
    std::memcpy(&result, static_cast<const std::uint8_t*>(object) + offset,
                sizeof(result));
    return result;
}
} // namespace

bool build_presentation_string_descriptor(const void* string_component,
                                          const float* parent_matrix,
                                          const wchar_t* resolved_text,
                                          BitmapCoordinateSpace space,
                                          PresentationStringDescriptor& out) {
    const auto options = field<std::uint8_t>(string_component, 0x60);
    if ((options & 4) == 0) {
        return false;
    }
    out = {};
    if (resolved_text) out.text = resolved_text;
    if (const auto* font = field<const char*>(string_component, 0x30)) {
        out.font = font;
    }
    out.colour = field<std::uint32_t>(string_component, 0x0c);
    std::memcpy(out.matrix.data(), parent_matrix, sizeof(float) * 16);
    for (std::size_t index = 0; index < out.rectangle.size(); ++index) {
        out.rectangle[index] = field<float>(string_component, 0x50 + index * 4);
    }
    out.line_height = field<float>(string_component, 0x34);
    out.line_spacing = field<float>(string_component, 0x38);
    const auto horizontal = field<std::uint32_t>(string_component, 0x40);
    out.horizontal_align = horizontal <= 3 ? horizontal : 0;
    const auto vertical = field<std::uint32_t>(string_component, 0x3c);
    out.vertical_align = vertical <= 2 ? vertical : 0;
    out.shadow_offset = field<std::uint32_t>(string_component, 0x48);
    out.shadow_colour = field<std::uint32_t>(string_component, 0x4c);
    out.option_bits = options & 3;
    out.space = space;
    return true;
}

} // namespace hp6::presentation
