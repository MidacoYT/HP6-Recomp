#include "presentation/PresentationStringRender.h"

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>

namespace {
template <typename T> void put(void* object, std::size_t offset, T value) {
    std::memcpy(static_cast<std::uint8_t*>(object) + offset, &value, sizeof(value));
}
} // namespace

int main() {
    std::array<std::uint8_t, 0x88> component{};
    const char font[] = "LS_Ascender_Uni_211";
    put(component.data(), 0x30, font + 0);
    put(component.data(), 0x0c, 0xd6d8b3ffu);
    put(component.data(), 0x34, 0.065f);
    put(component.data(), 0x38, 0.0f);
    put(component.data(), 0x3c, 1u);
    put(component.data(), 0x40, 5u);
    put(component.data(), 0x50, -0.2f);
    put(component.data(), 0x54, 0.2f);
    put(component.data(), 0x58, -0.1f);
    put(component.data(), 0x5c, 0.1f);
    std::array<float, 16> matrix{};
    matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1.0f;
    hp6::presentation::PresentationStringDescriptor result;
    const hp6::presentation::BitmapCoordinateSpace space{"WORLD", 2, 3};
    assert(!hp6::presentation::build_presentation_string_descriptor(
        component.data(), matrix.data(), L"English", space, result));
    put(component.data(), 0x60, std::uint8_t{4});
    assert(hp6::presentation::build_presentation_string_descriptor(
        component.data(), matrix.data(), L"English", space, result));
    assert(result.text == L"English");
    assert(result.font == font);
    assert(result.colour == 0xd6d8b3ffu);
    assert(std::fabs(result.line_height - 0.065f) < 0.00001f);
    assert(result.horizontal_align == 0); // the native switch accepts only 0..3
    assert(result.vertical_align == 1);
    assert(result.rectangle[0] == -0.2f && result.rectangle[3] == 0.1f);
    assert(result.matrix[15] == 1.0f && result.space.match_dword == 3);
}
