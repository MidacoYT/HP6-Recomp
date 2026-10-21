#pragma once

#include "presentation/LanguageSelectAssets.h"

#include <array>
#include <cstdint>
#include <string_view>

namespace hp6::presentation {

// Native SAS colour words are little-endian RGBA: red occupies bits 0..7.
struct SasColorChannels {
    std::array<const SasAnimationChannelAsset*, 4> rgba{};
};

SasColorChannels find_sas_color_channels(const SasAnimationBankAsset& bank,
                                         std::string_view clip);

// 0x5a0fe0: evaluate present colour curves and retain the missing channels.
std::uint32_t sample_sas_color_target(const SasColorChannels& channels,
                                      std::uint32_t original_rgba,
                                      float time);

// 0x576bc0: update once per timestamp. The four float deltas persist between
// updates; the returned error is the sum of normalized channel differences.
class SasColorSmoother {
public:
    SasColorSmoother(std::uint32_t initial_rgba = 0)
        : current_(initial_rgba) {}

    void reset(std::uint32_t rgba);
    float update(std::uint32_t target_rgba, std::uint64_t timestamp);
    std::uint32_t current() const { return current_; }

private:
    std::uint32_t current_ = 0;
    std::array<float, 4> delta_{};
    std::uint64_t last_timestamp_ = 0;
    bool has_timestamp_ = false;
};

} // namespace hp6::presentation
