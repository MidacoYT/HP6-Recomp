#include "presentation/BitmapRenderQuad.h"

#include <cstring>
#include <cstdlib>

#include "math/Transform.h"
#include "presentation/GuiTextTable.h"

extern float DAT_007db234; // shared identity/alpha constant; value supplied by game image

namespace hp6::presentation {
namespace {
template <typename T> T field(const std::uint8_t* object, std::size_t offset) {
    T value{};
    std::memcpy(&value, object + offset, sizeof(value));
    return value;
}
} // namespace

bool build_bitmap_render_quad(const void* bitmap, const float* camera_matrix,
                              const char* match_name, std::uint32_t match_word,
                              std::uint32_t match_dword, BitmapRenderQuad& out) {
    const auto* source = static_cast<const std::uint8_t*>(bitmap);
    if (source[0x58] == 0) {
        return false;
    }

    // 0056baa0 places this+0x10..+0x18 on the diagonal of a 4x4 local
    // transform, with +0x3c = 1, then calls 00401520. The camera selection
    // and 006236d0 branch are resolved by the caller of this producer.
    float local[16]{};
    local[0] = field<float>(source, 0x10);
    local[5] = field<float>(source, 0x14);
    local[10] = field<float>(source, 0x18);
    local[15] = DAT_007db234;
    // 0056baa0 calls 00401520(&local_50, &local_e0, &local_50): the
    // producer's local diagonal is the LEFT operand.
    hp6::math::multiply_matrix4x4(out.matrix, local, camera_matrix);

    auto& d = out.descriptor;
    d = {};
    d.matrix = out.matrix;
    d.value_20 = DAT_007db234;
    d.value_08 = field<std::uint32_t>(source, 0x0c);
    d.rect_x_left = d.x_left = field<float>(source, 0x48);
    d.rect_x_right = d.x_right = field<float>(source, 0x4c);
    d.rect_y_a = d.y_a = field<float>(source, 0x50);
    d.rect_y_b = d.y_b = field<float>(source, 0x54);

    const std::uint8_t options = source[0x28];
    // The assembly copies source bits 1, 2 and 4 into descriptor bits
    // 0, 1 and 2. Higher descriptor bits come from an uninitialised stack
    // byte in the original and have no reader in the integrated layout path.
    d.flags = static_cast<std::uint32_t>(((options >> 1) & 0x3) |
                                          ((options >> 2) & 0x4));
    if ((options & 8) != 0) {
        d.u_left = field<std::uint16_t>(source, 0x40);
        d.u_right = field<std::uint16_t>(source, 0x42);
        d.v_b = field<std::uint16_t>(source, 0x44);
        d.v_a = field<std::uint16_t>(source, 0x46);
    } else {
        d.u_left = d.v_b = 0;
        d.u_right = d.v_a = 0x1000;
    }
    d.default_bucket = field<std::uint16_t>(source, 0x38);
    d.value_3c = field<std::uint32_t>(source, 0x30);
    d.match_name = match_name;
    d.match_data_48 = match_word;
    d.match_dword = match_dword;

    const char* key = reinterpret_cast<const char*>(static_cast<std::uintptr_t>(
        field<std::uint32_t>(source, 0x2c)));
    out.fallback_text.clear();
    if (key != nullptr) {
        // 00429290 is the game's indirect UITextDb lookup. Its backing
        // table is already reconstructed by GuiTextTable.cpp. The original
        // then tries a second language map and finally mbstowcs; only the
        // first and final paths are available here.
        d.text = read_localized_text(0x00cf8168u, key);
        if (d.text == nullptr) {
            std::size_t length = std::mbstowcs(nullptr, key, 0);
            if (length != static_cast<std::size_t>(-1)) {
                out.fallback_text.resize(length);
                if (length != 0) {
                    std::mbstowcs(out.fallback_text.data(), key, length);
                }
                d.text = out.fallback_text.c_str();
            }
        }
    }
    return true;
}

bool build_bitmap_render_quad_mode2(const void* bitmap,
                                    const BitmapCoordinateSpace* spaces,
                                    BitmapRenderQuad& out) {
    const auto* source = static_cast<const std::uint8_t*>(bitmap);
    if (source[0x58] == 0) {
        return false;
    }
    const auto parent_address = field<std::uint32_t>(source, 0x04);
    const auto* parent = reinterpret_cast<const std::uint8_t*>(
        static_cast<std::uintptr_t>(parent_address));
    const auto space_index = field<std::uint32_t>(source, 0x34);
    const auto& space = spaces[space_index];
    return build_bitmap_render_quad(bitmap,
        reinterpret_cast<const float*>(parent + 0x10),
        space.name, space.match_word, space.match_dword, out);
}

} // namespace hp6::presentation
