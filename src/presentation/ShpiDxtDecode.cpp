#include "presentation/ShpiDxtDecode.h"

#include <array>
#include <cstddef>
#include <stdexcept>

namespace hp6::presentation {
namespace {
using Pixel = std::array<std::uint8_t, 4>;

std::uint16_t half(const std::uint8_t* p) {
    return std::uint16_t(p[0]) | std::uint16_t(p[1]) << 8;
}

std::uint32_t word(const std::uint8_t* p) {
    return std::uint32_t(p[0]) | std::uint32_t(p[1]) << 8 |
           std::uint32_t(p[2]) << 16 | std::uint32_t(p[3]) << 24;
}

Pixel colour_565(std::uint16_t value) {
    const auto r = (value >> 11) & 31;
    const auto g = (value >> 5) & 63;
    const auto b = value & 31;
    return {static_cast<std::uint8_t>((r << 3) | (r >> 2)),
            static_cast<std::uint8_t>((g << 2) | (g >> 4)),
            static_cast<std::uint8_t>((b << 3) | (b >> 2)), 255};
}

std::array<Pixel, 4> colour_palette(const std::uint8_t* p,
                                    bool one_bit_alpha) {
    const auto c0 = half(p);
    const auto c1 = half(p + 2);
    std::array<Pixel, 4> palette{};
    palette[0] = colour_565(c0);
    palette[1] = colour_565(c1);
    if (one_bit_alpha && c0 <= c1) {
        for (int channel = 0; channel < 3; ++channel) {
            palette[2][channel] = static_cast<std::uint8_t>(
                (int(palette[0][channel]) + palette[1][channel]) / 2);
        }
        palette[2][3] = 255;
        palette[3] = {0, 0, 0, 0};
    } else {
        for (int channel = 0; channel < 3; ++channel) {
            palette[2][channel] = static_cast<std::uint8_t>(
                (2 * int(palette[0][channel]) + palette[1][channel]) / 3);
            palette[3][channel] = static_cast<std::uint8_t>(
                (int(palette[0][channel]) + 2 * palette[1][channel]) / 3);
        }
        palette[2][3] = palette[3][3] = 255;
    }
    return palette;
}

std::array<std::uint8_t, 8> alpha_palette(const std::uint8_t* block) {
    std::array<std::uint8_t, 8> palette{};
    palette[0] = block[0];
    palette[1] = block[1];
    if (palette[0] > palette[1]) {
        for (int i = 1; i <= 6; ++i) {
            palette[i + 1] = static_cast<std::uint8_t>(
                ((7 - i) * int(palette[0]) + i * int(palette[1])) / 7);
        }
    } else {
        for (int i = 1; i <= 4; ++i) {
            palette[i + 1] = static_cast<std::uint8_t>(
                ((5 - i) * int(palette[0]) + i * int(palette[1])) / 5);
        }
        palette[6] = 0;
        palette[7] = 255;
    }
    return palette;
}
} // namespace

std::vector<std::uint8_t> decode_shpi_mip0_rgba(
    const LanguageSelectTextureAsset& texture) {
    if (texture.width == 0 || texture.height == 0 ||
        texture.format < 0x60 || texture.format > 0x62) {
        throw std::invalid_argument("SHPI: unsupported mip zero");
    }
    const auto blocks_x = (std::size_t(texture.width) + 3) / 4;
    const auto blocks_y = (std::size_t(texture.height) + 3) / 4;
    const auto block_size = texture.format == 0x60 ? 8u : 16u;
    if (texture.mip0_blocks.size() != blocks_x * blocks_y * block_size) {
        throw std::invalid_argument("SHPI: wrong mip zero byte count");
    }
    std::vector<std::uint8_t> rgba(
        std::size_t(texture.width) * texture.height * 4);
    for (std::size_t by = 0; by < blocks_y; ++by) {
        for (std::size_t bx = 0; bx < blocks_x; ++bx) {
            const auto* block = texture.mip0_blocks.data() +
                                (by * blocks_x + bx) * block_size;
            const auto* colour = block + (texture.format == 0x60 ? 0 : 8);
            const auto palette = colour_palette(colour, texture.format == 0x60);
            const auto colour_bits = word(colour + 4);
            const auto alpha = texture.format == 0x62 ?
                alpha_palette(block) : std::array<std::uint8_t, 8>{};
            std::uint64_t alpha_bits = 0;
            if (texture.format == 0x62) {
                for (int byte = 0; byte < 6; ++byte) {
                    alpha_bits |= std::uint64_t(block[2 + byte]) << (byte * 8);
                }
            }
            for (std::size_t y = 0; y < 4; ++y) {
                for (std::size_t x = 0; x < 4; ++x) {
                    const auto px = bx * 4 + x;
                    const auto py = by * 4 + y;
                    if (px >= texture.width || py >= texture.height) continue;
                    const auto index = y * 4 + x;
                    const auto colour_index = (colour_bits >> (index * 2)) & 3;
                    auto pixel = palette[colour_index];
                    if (texture.format == 0x61) {
                        const auto packed = block[index / 2];
                        pixel[3] = static_cast<std::uint8_t>(
                            ((packed >> ((index % 2) * 4)) & 15) * 17);
                    } else if (texture.format == 0x62) {
                        pixel[3] = alpha[(alpha_bits >> (index * 3)) & 7];
                    }
                    const auto offset = (py * texture.width + px) * 4;
                    for (int channel = 0; channel < 4; ++channel) {
                        rgba[offset + channel] = pixel[channel];
                    }
                }
            }
        }
    }
    return rgba;
}

} // namespace hp6::presentation
