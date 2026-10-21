#include "presentation/BitmapRenderQuad.h"
#include "presentation/GuiTextTable.h"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <cwchar>

float DAT_007db234 = 1.0f;

template <typename T>
void put(std::uint8_t* bytes, std::size_t offset, T value) {
    std::memcpy(bytes + offset, &value, sizeof(value));
}

int main() {
    static_assert(sizeof(void*) == 4, "the original bitmap layout is x86");
    std::uint8_t bitmap[0x5c]{};
    float camera[16]{};
    camera[0] = camera[5] = camera[10] = camera[15] = 1;
    camera[12] = 7;
    bitmap[0x58] = 1;
    bitmap[0x28] = 0x1e; // text mode, explicit UVs
    put(bitmap, 0x10, 2.0f);
    put(bitmap, 0x14, 3.0f);
    put(bitmap, 0x18, 4.0f);
    put(bitmap, 0x48, -1.0f);
    put(bitmap, 0x4c, 1.0f);
    put(bitmap, 0x50, -0.5f);
    put(bitmap, 0x54, 0.5f);
    put(bitmap, 0x40, std::uint16_t{12});
    put(bitmap, 0x42, std::uint16_t{34});
    put(bitmap, 0x44, std::uint16_t{56});
    put(bitmap, 0x46, std::uint16_t{78});
    put(bitmap, 0x38, std::uint16_t{91});
    put(bitmap, 0x30, std::uint32_t{0x12345678});
    const char* key = "GUI_LANGUAGEMENU0";
    put(bitmap, 0x2c, static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(key)));

    hp6::presentation::update_localized_text(0x00cf8168u, key, L"Francais");
    hp6::presentation::BitmapRenderQuad quad;
    assert(hp6::presentation::build_bitmap_render_quad(
        bitmap, camera, "WORLD", 2, 3, quad));
    const auto& d = quad.descriptor;
    assert(d.text && std::wcscmp(d.text, L"Francais") == 0);
    assert(d.flags == 7 && d.matrix == quad.matrix);
    assert(d.x_left == -1 && d.rect_x_right == 1 && d.y_a == -0.5f);
    assert(d.u_left == 12 && d.u_right == 34 && d.v_b == 56 && d.v_a == 78);
    assert(d.default_bucket == 91 && d.match_data_48 == 2 && d.match_dword == 3);
    assert(d.value_20 == DAT_007db234 && d.value_3c == 0x12345678);
    assert(quad.matrix[0] == 2 && quad.matrix[5] == 3 && quad.matrix[10] == 4);
    assert(quad.matrix[12] == 7); // local * camera, not camera * local

    bitmap[0x28] = 0x10;
    assert(hp6::presentation::build_bitmap_render_quad(
        bitmap, camera, "WORLD", 2, 3, quad));
    assert(quad.descriptor.flags == 4);

    std::uint8_t parent[0x50]{};
    std::memcpy(parent + 0x10, camera, sizeof(camera));
    put(bitmap, 0x04, static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(parent)));
    put(bitmap, 0x34, std::uint32_t{1});
    const hp6::presentation::BitmapCoordinateSpace spaces[] = {
        {"SCREEN", 4, 5}, {"WORLD", 8, 9}};
    assert(hp6::presentation::build_bitmap_render_quad_mode2(
        bitmap, spaces, quad));
    assert(quad.descriptor.match_name == spaces[1].name &&
           quad.descriptor.match_data_48 == 8 &&
           quad.descriptor.match_dword == 9 &&
           quad.matrix[12] == 7);

    bitmap[0x28] = 0;
    assert(hp6::presentation::build_bitmap_render_quad(
        bitmap, camera, "WORLD", 2, 3, quad));
    assert(quad.descriptor.flags == 0 && quad.descriptor.u_right == 0x1000);
    bitmap[0x58] = 0;
    assert(!hp6::presentation::build_bitmap_render_quad(
        bitmap, camera, "WORLD", 2, 3, quad));
}
