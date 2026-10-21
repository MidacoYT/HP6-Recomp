#include "presentation/LanguageSelectAssets.h"
#include "presentation/LanguageSelectNames.h"
#include "presentation/LanguageSkuConfig.h"
#include "presentation/PresentationFocusRegistry.h"
#include "presentation/SasColorAnimation.h"
#include "presentation/PresentationStringLayout.h"
#include "presentation/ShpiDxtDecode.h"
#include "math/Transform.h"

#define NOMINMAX
#include <windows.h>
#include <shellapi.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace {
constexpr int width = 1280;
constexpr int height = 720;
using hp6::presentation::LanguageSelectAssets;
using hp6::presentation::LanguageSelectFontAsset;
using hp6::presentation::LanguageSelectTextAsset;

// A diagnostic renderer. The real menu still needs its D3D scene and native
// language-service order; do not present this window as the reconstructed game.

struct Menu {
    LanguageSelectAssets assets;
    hp6::presentation::LanguageSelectHighlightAssets highlight;
    hp6::presentation::SasAnimationBankAsset animation_bank;
    hp6::presentation::SasColorChannels focus_colour_channels;
    std::array<hp6::presentation::SasColorSmoother, 16> button_colours{};
    std::uint64_t focus_started_ms = 0;
    std::vector<std::uint32_t> pixels;
    std::vector<std::uint8_t> glow_pixels;
    std::vector<std::uint8_t> underline_pixels;
    std::vector<std::uint8_t> bracket_pixels;
    std::vector<std::uint8_t> return_icon_pixels;
    hp6::presentation::PresentationFocusRegistry focus_registry;
    std::array<std::unique_ptr<
        hp6::presentation::PresentationFocusLocatorMovable>, 16> locators{};
    std::array<std::unique_ptr<
        hp6::presentation::PresentationFocusPhase>, 16> focus_phases{};
    int locator_selection = -1;
    std::array<const hp6::presentation::LanguageSelectName*, 16> names{};
    int language_count = 16;
    int selection = 1;
    int cursor_x = width / 2;
    int cursor_y = height / 2;
};

std::uint32_t authored_rgba(const LanguageSelectTextAsset& asset) {
    const auto colour = asset.font_colour; // serialized RRGGBBAA
    return (colour >> 24) | ((colour >> 8) & 0xff00u) |
           ((colour << 8) & 0xff0000u) | (colour << 24);
}

void start_focus_colour(Menu& menu, int old_selection,
                        std::uint64_t timestamp_ms) {
    if (old_selection >= 0) {
        menu.button_colours[old_selection].reset(
            authored_rgba(menu.assets.texts[old_selection]));
    }
    menu.button_colours[menu.selection].reset(
        authored_rgba(menu.assets.texts[menu.selection]));
    menu.focus_started_ms = timestamp_ms;
}

void advance_focus_colour(Menu& menu, float seconds,
                          std::uint64_t timestamp) {
    const int selected = menu.selection;
    const auto target = hp6::presentation::sample_sas_color_target(
        menu.focus_colour_channels,
        authored_rgba(menu.assets.texts[selected]), seconds);
    menu.button_colours[selected].update(target, timestamp);
}

void set_sku_order(Menu& menu, const hp6::presentation::LanguageSkuConfig& sku) {
    menu.names.fill(nullptr);
    menu.language_count = std::min<int>(16, sku.languages.size());
    if (menu.language_count == 0) {
        throw std::runtime_error("SKU config has no LANGUAGE records");
    }
    for (int index = 0; index < menu.language_count; ++index) {
        std::string locale = sku.languages[index];
        std::transform(locale.begin(), locale.end(), locale.begin(),
                       [](unsigned char c) { return char(std::tolower(c)); });
        for (const auto& name : hp6::presentation::kInstalledLanguageNames) {
            if (locale == name.locale) {
                menu.names[index] = &name;
                break;
            }
        }
    }
    menu.selection = menu.language_count > 1 ? 1 : 0;
}

std::filesystem::path find_archive(int argc, wchar_t** argv) {
    if (argc >= 2 && argv[1][0] != L'-') return argv[1];
    for (auto base = std::filesystem::current_path(); !base.empty();
         base = base.parent_path()) {
        const auto candidate = base / "extract/chapters/languageselect.str";
        if (std::filesystem::exists(candidate)) return candidate;
        const auto sibling = base.parent_path() / "extract/chapters/languageselect.str";
        if (std::filesystem::exists(sibling)) return sibling;
        if (base == base.parent_path()) break;
    }
    throw std::runtime_error("languageselect.str not found");
}

std::array<float, 3> project(const LanguageSelectAssets& assets,
                             const std::array<float, 3>& position,
                             float focal_length = 768.0f) {
    std::array<float, 3> camera{};
    for (int axis = 0; axis < 3; ++axis) {
        float value = 0;
        for (int k = 0; k < 3; ++k) {
            value += (position[k] - assets.camera_position[k]) *
                     assets.camera_basis[axis * 4 + k];
        }
        camera[axis] = value;
    }
    // Camera pose is original. Focal length is fitted to the supplied frame;
    // its exact native source remains to be traced.
    const float scale = focal_length / -camera[2];
    return {width * 0.5f + camera[0] * scale,
            height * 0.5f - camera[1] * scale, scale};
}

void fill(Menu& menu) {
    menu.pixels.assign(width * height, 0xff000000u);
}

void blend(Menu& menu, int x, int y, std::uint8_t alpha,
           std::uint32_t colour) {
    if (x < 0 || x >= width || y < 0 || y >= height || !alpha) return;
    auto& dst = menu.pixels[y * width + x];
    const int r = (((dst >> 16) & 255) * (255 - alpha) +
                   ((colour >> 16) & 255) * alpha) / 255;
    const int g = (((dst >> 8) & 255) * (255 - alpha) +
                   ((colour >> 8) & 255) * alpha) / 255;
    const int b = ((dst & 255) * (255 - alpha) +
                   (colour & 255) * alpha) / 255;
    dst = std::uint32_t(r) << 16 | std::uint32_t(g) << 8 | std::uint32_t(b);
}

void draw_glyph(Menu& menu, const LanguageSelectFontAsset& font,
                wchar_t character, float x, float baseline,
                float scale, std::uint32_t colour) {
    auto found = font.glyphs.find(static_cast<std::uint16_t>(character));
    if (found == font.glyphs.end()) return;
    const auto& glyph = found->second;
    const int left = static_cast<int>(std::lround(x));
    const int top = static_cast<int>(std::lround(baseline +
                  glyph.top_offset * scale));
    const int draw_width = static_cast<int>(std::ceil(glyph.width * scale));
    const int draw_height = static_cast<int>(std::ceil(glyph.height * scale));
    for (int py = 0; py < draw_height; ++py) {
        const int source_y = std::min(glyph.height - 1,
            static_cast<int>(py / scale));
        for (int px = 0; px < draw_width; ++px) {
            const int source_x = std::min(glyph.width - 1,
                static_cast<int>(px / scale));
            const auto alpha = font.alpha[
                (glyph.atlas_v + source_y) * font.atlas_width +
                glyph.atlas_u + source_x];
            blend(menu, left + px, top + py,
                  static_cast<std::uint8_t>(std::min(255, int(alpha) * 2)),
                  colour);
        }
    }
}

void draw_text(Menu& menu, const wchar_t* label,
               const LanguageSelectTextAsset& asset,
               std::uint32_t packed_rgba) {
    const auto& font = menu.assets.font;
    const auto shaped = hp6::presentation::shape_plain_presentation_string(
        label, font);
    auto glyphs = shaped.glyphs;
    const auto length = [](const std::array<float, 12>& basis, int row) {
        return std::sqrt(basis[row] * basis[row] +
                         basis[row + 1] * basis[row + 1] +
                         basis[row + 2] * basis[row + 2]);
    };
    const float x_length = length(asset.basis, 0);
    const float y_length = length(asset.basis, 4);
    hp6::presentation::PresentationStringDescriptor descriptor;
    descriptor.line_height = asset.line_height;
    descriptor.rectangle = {-x_length * 0.5f, x_length * 0.5f,
                            -y_length * 0.5f, y_length * 0.5f};
    descriptor.horizontal_align = asset.horizontal_align <= 3 ?
        asset.horizontal_align : 0;
    descriptor.vertical_align = asset.vertical_align <= 2 ?
        asset.vertical_align : 0;
    hp6::presentation::layout_presentation_string_glyphs(
        glyphs, shaped.lines, descriptor, float(font.metric_height));
    const std::uint32_t colour = ((packed_rgba & 0xffu) << 16) |
        (packed_rgba & 0xff00u) | ((packed_rgba >> 16) & 0xffu);
    const float scale = asset.line_height *
        project(menu.assets, asset.position)[2] / font.metric_height;
    for (std::size_t index = 0; label[index] != 0; ++index) {
        const wchar_t character = label[index];
        if (character == L' ' || character == L'\t') continue;
        const auto found = font.glyphs.find(static_cast<std::uint16_t>(character));
        if (found == font.glyphs.end()) continue;
        const auto& glyph = glyphs[index];
        std::array<float, 3> world{};
        for (int axis = 0; axis < 3; ++axis) {
            world[axis] = asset.position[axis] +
                glyph.x_left * asset.basis[axis] / x_length +
                glyph.y_top * asset.basis[4 + axis] / y_length;
        }
        const auto screen = project(menu.assets, world);
        const float baseline = screen[1] - found->second.top_offset * scale;
        draw_glyph(menu, font, character, screen[0], baseline, scale, colour);
    }
}

hp6::presentation::PresentationMatrix asset_matrix(
    const std::array<float, 12>& basis,
    const std::array<float, 3>& position) {
    hp6::presentation::PresentationMatrix result{};
    std::copy(basis.begin(), basis.end(), result.begin());
    std::copy(position.begin(), position.end(), result.begin() + 12);
    result[15] = 1.0f;
    return result;
}

void draw_highlight_bitmap(Menu& menu,
                           const std::array<float, 12>& authored_basis,
                           const std::array<float, 3>& authored_position,
                           const hp6::presentation::LanguageSelectTextureAsset& texture,
                           const std::vector<std::uint8_t>& rgba,
                           const hp6::presentation::PresentationMatrix& follower,
                           bool flip_u) {
    const auto authored = asset_matrix(authored_basis, authored_position);
    hp6::presentation::PresentationMatrix placed{};
    hp6::math::multiply_matrix4x4(placed.data(), authored.data(), follower.data());
    const auto corner = [&](float local_x, float local_y) {
        std::array<float, 3> world{};
        for (int axis = 0; axis < 3; ++axis) {
            world[axis] = placed[12 + axis] +
                local_x * placed[axis] + local_y * placed[4 + axis];
        }
        return project(menu.assets, world, 750.0f);
    };
    // The runtime bitmap initializer uses [-0.5,+0.5] on both local axes.
    // The screenshot resolves the U orientation of these two source textures.
    const auto top_left = corner(0.5f, 0.5f);
    const auto top_right = corner(-0.5f, 0.5f);
    const auto bottom_left = corner(0.5f, -0.5f);
    const auto bottom_right = corner(-0.5f, -0.5f);
    const int min_x = std::max(0, int(std::floor(std::min({
        top_left[0], top_right[0], bottom_left[0], bottom_right[0]}))));
    const int max_x = std::min(width - 1, int(std::ceil(std::max({
        top_left[0], top_right[0], bottom_left[0], bottom_right[0]}))));
    const int min_y = std::max(0, int(std::floor(std::min({
        top_left[1], top_right[1], bottom_left[1], bottom_right[1]}))));
    const int max_y = std::min(height - 1, int(std::ceil(std::max({
        top_left[1], top_right[1], bottom_left[1], bottom_right[1]}))));
    const float ux = top_right[0] - top_left[0];
    const float uy = top_right[1] - top_left[1];
    const float vx = bottom_left[0] - top_left[0];
    const float vy = bottom_left[1] - top_left[1];
    const float determinant = ux * vy - uy * vx;
    if (std::fabs(determinant) < 0.001f) return;
    for (int y = min_y; y <= max_y; ++y) {
        for (int x = min_x; x <= max_x; ++x) {
            const float dx = x + 0.5f - top_left[0];
            const float dy = y + 0.5f - top_left[1];
            const float u = (dx * vy - dy * vx) / determinant;
            const float v = (dy * ux - dx * uy) / determinant;
            if (u < 0 || u >= 1 || v < 0 || v >= 1) continue;
            const int tx = std::min(int(texture.width - 1),
                int((flip_u ? 1.0f - u : u) * texture.width));
            const int ty = std::min(int(texture.height - 1), int(v * texture.height));
            const std::size_t offset = (std::size_t(ty) * texture.width + tx) * 4;
            const auto colour = (std::uint32_t(rgba[offset]) << 16) |
                (std::uint32_t(rgba[offset + 1]) << 8) | rgba[offset + 2];
            blend(menu, x, y, rgba[offset + 3], colour);
        }
    }
}

void draw_highlight(Menu& menu) {
    const auto& marker = menu.assets.markers[menu.selection];
    if (menu.locator_selection < 0) {
        for (int index = 0; index < 16; ++index) {
            const auto& source = menu.assets.markers[index];
            menu.locators[index] = std::make_unique<
                hp6::presentation::PresentationFocusLocatorMovable>(
                    menu.focus_registry, source.resource,
                    asset_matrix(source.basis, source.position));
            menu.focus_phases[index] = std::make_unique<
                hp6::presentation::PresentationFocusPhase>(*menu.locators[index]);
        }
        menu.focus_phases[menu.selection]->request_focus();
        // The diagnostic renderer has no native animation owner yet. Its
        // completion callback therefore resolves on this render pass.
        menu.focus_phases[menu.selection]->tick(true);
        menu.locator_selection = menu.selection;
    } else if (menu.locator_selection != menu.selection) {
        menu.focus_phases[menu.locator_selection]->request_blur();
        menu.focus_phases[menu.locator_selection]->tick(true);
        menu.focus_phases[menu.selection]->request_focus();
        menu.focus_phases[menu.selection]->tick(true);
        menu.locator_selection = menu.selection;
    }
    menu.locators[menu.selection]->update(
        asset_matrix(marker.basis, marker.position));
    const auto inverse = hp6::presentation::make_presentation_focus_follower_local(
        asset_matrix(menu.highlight.follower_basis,
                     menu.highlight.follower_position));
    const auto follower = hp6::presentation::compose_presentation_focus_follower(
        menu.focus_registry, menu.highlight.locator_name, inverse);
    draw_highlight_bitmap(menu, menu.highlight.underline_basis,
        menu.highlight.underline_position, menu.highlight.underline,
        menu.underline_pixels, follower, true);
    draw_highlight_bitmap(menu, menu.highlight.bracket_basis,
        menu.highlight.bracket_position, menu.highlight.bracket,
        menu.bracket_pixels, follower, false);
    const auto& source = menu.highlight.glyph_position;
    std::array<float, 3> placed{};
    for (int axis = 0; axis < 3; ++axis) {
        placed[axis] = follower[12 + axis] + source[0] * follower[axis] +
            source[1] * follower[4 + axis] + source[2] * follower[8 + axis];
    }
    const auto glyph_screen = project(menu.assets, placed, 750.0f);
    const int icon_size = int(std::lround(
        menu.highlight.glyph_line_height * glyph_screen[2]));
    const int icon_left = int(std::lround(glyph_screen[0])) - icon_size - 8;
    const int icon_top = int(std::lround(glyph_screen[1])) - icon_size / 2;
    const auto& icon = menu.assets.keyboard_select_icon;
    for (int y = 0; y < icon_size; ++y) {
        for (int x = 0; x < icon_size; ++x) {
            const int tx = x * icon.width / icon_size;
            const int ty = y * icon.height / icon_size;
            const std::size_t offset = (std::size_t(ty) * icon.width + tx) * 4;
            const auto colour =
                (std::uint32_t(menu.return_icon_pixels[offset]) << 16) |
                (std::uint32_t(menu.return_icon_pixels[offset + 1]) << 8) |
                menu.return_icon_pixels[offset + 2];
            blend(menu, icon_left + x, icon_top + y,
                  menu.return_icon_pixels[offset + 3], colour);
        }
    }
}

void draw_cursor(Menu& menu) {
    // ET_glow_lamp belongs to the original chapter and visually matches the
    // captured cursor. Its runtime binding still needs to be traced.
    const auto& texture = menu.assets.glow_lamp;
    const int left = menu.cursor_x - texture.width / 2;
    const int top = menu.cursor_y - texture.height / 2;
    for (int y = 0; y < texture.height; ++y) {
        for (int x = 0; x < texture.width; ++x) {
            const std::size_t offset = (std::size_t(y) * texture.width + x) * 4;
            const auto alpha = menu.glow_pixels[offset + 3];
            const auto colour = (std::uint32_t(menu.glow_pixels[offset]) << 16) |
                (std::uint32_t(menu.glow_pixels[offset + 1]) << 8) |
                menu.glow_pixels[offset + 2];
            blend(menu, left + x, top + y, alpha, colour);
        }
    }
}

void render(Menu& menu) {
    fill(menu);
    draw_highlight(menu);
    for (int i = 0; i < 16; ++i) {
        const auto& text_asset = menu.assets.texts[i];
        if (menu.names[i]) {
            draw_text(menu, menu.names[i]->display,
                      text_asset, menu.button_colours[i].current());
        }
    }
    draw_cursor(menu);
}

void save_bmp(const Menu& menu, const std::filesystem::path& path) {
    BITMAPFILEHEADER file{};
    BITMAPINFOHEADER info{};
    file.bfType = 0x4d42;
    file.bfOffBits = sizeof(file) + sizeof(info);
    file.bfSize = file.bfOffBits + width * height * 4;
    info.biSize = sizeof(info);
    info.biWidth = width;
    info.biHeight = -height;
    info.biPlanes = 1;
    info.biBitCount = 32;
    info.biCompression = BI_RGB;
    std::ofstream output(path, std::ios::binary);
    if (!output) throw std::runtime_error("cannot create menu snapshot");
    output.write(reinterpret_cast<const char*>(&file), sizeof(file));
    output.write(reinterpret_cast<const char*>(&info), sizeof(info));
    output.write(reinterpret_cast<const char*>(menu.pixels.data()),
                 width * height * 4);
}

LRESULT CALLBACK window_proc(HWND hwnd, UINT message, WPARAM wparam,
                             LPARAM lparam) {
    auto* menu = reinterpret_cast<Menu*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    switch (message) {
    case WM_SETCURSOR:
        if (LOWORD(lparam) == HTCLIENT) {
            SetCursor(nullptr);
            return TRUE;
        }
        return DefWindowProcW(hwnd, message, wparam, lparam);
    case WM_MOUSEMOVE:
        if (menu) {
            RECT client{};
            GetClientRect(hwnd, &client);
            if (client.right > 0 && client.bottom > 0) {
                menu->cursor_x = std::clamp(
                    int(LOWORD(lparam)) * width / int(client.right), 0, width - 1);
                menu->cursor_y = std::clamp(
                    int(HIWORD(lparam)) * height / int(client.bottom), 0, height - 1);
                render(*menu);
                InvalidateRect(hwnd, nullptr, FALSE);
            }
        }
        return 0;
    case WM_KEYDOWN:
        if (menu) {
            const int old = menu->selection;
            switch (wparam) {
            case VK_UP:
                menu->selection = menu->language_count == 16 ?
                    (old + 7) % 8 + (old / 8) * 8 :
                    (old + menu->language_count - 1) % menu->language_count;
                break;
            case VK_DOWN:
                menu->selection = menu->language_count == 16 ?
                    (old + 1) % 8 + (old / 8) * 8 :
                    (old + 1) % menu->language_count;
                break;
            case VK_LEFT: case VK_RIGHT:
                menu->selection = menu->language_count == 16 ?
                    (old + 8) % 16 : (old + 1) % menu->language_count;
                break;
            case VK_ESCAPE: DestroyWindow(hwnd); return 0;
            case VK_RETURN:
                MessageBoxW(hwnd,
                    menu->names[old] ? menu->names[old]->display : L" ",
                    L"Diagnostic language preview", MB_OK);
                return 0;
            default: break;
            }
            if (old != menu->selection) {
                start_focus_colour(*menu, old, GetTickCount64());
                render(*menu);
                InvalidateRect(hwnd, nullptr, FALSE);
            }
        }
        return 0;
    case WM_TIMER:
        if (menu && wparam == 1) {
            const auto now = GetTickCount64();
            advance_focus_colour(*menu,
                float(now - menu->focus_started_ms) * 0.001f, now);
            render(*menu);
            InvalidateRect(hwnd, nullptr, FALSE);
        }
        return 0;
    case WM_PAINT:
        if (menu) {
            PAINTSTRUCT paint{};
            const HDC dc = BeginPaint(hwnd, &paint);
            BITMAPINFO bmi{};
            bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
            bmi.bmiHeader.biWidth = width;
            bmi.bmiHeader.biHeight = -height;
            bmi.bmiHeader.biPlanes = 1;
            bmi.bmiHeader.biBitCount = 32;
            bmi.bmiHeader.biCompression = BI_RGB;
            RECT client{};
            GetClientRect(hwnd, &client);
            StretchDIBits(dc, 0, 0, client.right, client.bottom,
                          0, 0, width, height, menu->pixels.data(),
                          &bmi, DIB_RGB_COLORS, SRCCOPY);
            EndPaint(hwnd, &paint);
        }
        return 0;
    case WM_DESTROY: KillTimer(hwnd, 1); PostQuitMessage(0); return 0;
    default: return DefWindowProcW(hwnd, message, wparam, lparam);
    }
}
} // namespace

int WINAPI wWinMain(HINSTANCE instance, HINSTANCE, PWSTR, int show) {
    int argc = 0;
    wchar_t** argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (!argv) return 1;
    try {
        const auto path = find_archive(argc, argv);
        std::ifstream input(path, std::ios::binary);
        if (!input) throw std::runtime_error("cannot read languageselect.str");
        const std::vector<std::uint8_t> bytes(
            std::istreambuf_iterator<char>{input}, std::istreambuf_iterator<char>{});
        Menu menu{hp6::presentation::read_language_select_assets(bytes)};
        for (int i = 0; i < 16; ++i) {
            menu.names[i] = &hp6::presentation::kInstalledLanguageNames[i];
        }
        for (int i = 1; i + 1 < argc; ++i) {
            if (std::wstring_view(argv[i]) != L"--sku") continue;
            std::ifstream sku_input(argv[i + 1], std::ios::binary);
            if (!sku_input) throw std::runtime_error("cannot read SKU config");
            const std::string sku_bytes(
                std::istreambuf_iterator<char>{sku_input},
                std::istreambuf_iterator<char>{});
            set_sku_order(menu,
                hp6::presentation::parse_language_sku_config(sku_bytes));
            break;
        }
        const auto game_path = path.parent_path().parent_path() / "game.str";
        std::ifstream game_input(game_path, std::ios::binary);
        if (!game_input) throw std::runtime_error("cannot read game.str");
        const std::vector<std::uint8_t> game_bytes(
            std::istreambuf_iterator<char>{game_input},
            std::istreambuf_iterator<char>{});
        menu.highlight = hp6::presentation::read_language_select_highlight_assets(
            game_bytes);
        menu.animation_bank = hp6::presentation::read_sas_animation_bank(
            game_bytes, menu.assets.texts[0].animation_bank_hash);
        menu.focus_colour_channels =
            hp6::presentation::find_sas_color_channels(
                menu.animation_bank, menu.assets.texts[0].animation_clips[5]);
        for (int i = 0; i < 16; ++i) {
            menu.button_colours[i].reset(authored_rgba(menu.assets.texts[i]));
        }
        menu.button_colours[menu.selection].reset(0xffffffffu);
        menu.focus_started_ms = GetTickCount64() - 750;
        menu.underline_pixels = hp6::presentation::decode_shpi_mip0_rgba(
            menu.highlight.underline);
        menu.bracket_pixels = hp6::presentation::decode_shpi_mip0_rgba(
            menu.highlight.bracket);
        menu.return_icon_pixels = hp6::presentation::decode_shpi_mip0_rgba(
            menu.assets.keyboard_select_icon);
        menu.glow_pixels = hp6::presentation::decode_shpi_mip0_rgba(
            menu.assets.glow_lamp);
        render(menu);
        for (int i = 1; i + 1 < argc; ++i) {
            if (std::wstring_view(argv[i]) != L"--snapshot-time") continue;
            const float seconds = std::stof(std::wstring(argv[i + 1]));
            if (seconds < 0.0f || seconds > 10.0f || !std::isfinite(seconds)) {
                throw std::runtime_error("snapshot time must be 0..10 seconds");
            }
            start_focus_colour(menu, -1, 0);
            std::uint64_t frame = 1;
            for (float sample_time = 0.0f; sample_time < seconds;
                 sample_time += 1.0f / 30.0f) {
                advance_focus_colour(menu, sample_time, frame++);
            }
            advance_focus_colour(menu, seconds, frame);
            render(menu);
            break;
        }
        for (int i = 1; i + 1 < argc; ++i) {
            if (std::wstring_view(argv[i]) == L"--snapshot") {
                save_bmp(menu, argv[i + 1]);
                LocalFree(argv);
                return 0;
            }
        }
        WNDCLASSW wc{};
        wc.lpfnWndProc = window_proc;
        wc.hInstance = instance;
        wc.lpszClassName = L"HP6LanguageSelectMenu";
        wc.hCursor = LoadCursorW(nullptr, MAKEINTRESOURCEW(32512));
        RegisterClassW(&wc);
        RECT bounds{0, 0, width, height};
        AdjustWindowRect(&bounds, WS_OVERLAPPEDWINDOW, FALSE);
        HWND hwnd = CreateWindowExW(0, wc.lpszClassName,
            L"HP6 - diagnostic language preview (not original rendering)",
            WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
            bounds.right - bounds.left, bounds.bottom - bounds.top,
            nullptr, nullptr, instance, nullptr);
        if (!hwnd) throw std::runtime_error("cannot create menu window");
        SetWindowLongPtrW(hwnd, GWLP_USERDATA,
                          reinterpret_cast<LONG_PTR>(&menu));
        ShowWindow(hwnd, show);
        SetTimer(hwnd, 1, 33, nullptr);
        UpdateWindow(hwnd);
        MSG message{};
        while (GetMessageW(&message, nullptr, 0, 0) > 0) {
            TranslateMessage(&message);
            DispatchMessageW(&message);
        }
        LocalFree(argv);
        return static_cast<int>(message.wParam);
    } catch (const std::exception& error) {
        MessageBoxA(nullptr, error.what(), "HP6 language menu", MB_ICONERROR);
        LocalFree(argv);
        return 1;
    }
}
