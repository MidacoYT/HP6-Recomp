#pragma once

#include <cstdint>
#include <vector>

#include "presentation/LanguageSelectAssets.h"

namespace hp6::presentation {

// Decode mip zero of the original SHPI S3TC block payload to row-major RGBA8.
// This is an offline/CPU path; the D3D renderer can upload the original blocks.
std::vector<std::uint8_t> decode_shpi_mip0_rgba(
    const LanguageSelectTextureAsset& texture);

} // namespace hp6::presentation
