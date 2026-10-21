#include "presentation/SasColorAnimation.h"

#include "presentation/SasCurveEvaluator.h"

#include <cmath>
#include <string>

namespace hp6::presentation {
namespace {

std::uint8_t byte_at(std::uint32_t value, std::size_t component) {
    return std::uint8_t(value >> (component * 8));
}

} // namespace

SasColorChannels find_sas_color_channels(const SasAnimationBankAsset& bank,
                                         std::string_view clip) {
    SasColorChannels result;
    constexpr std::array<char, 4> suffixes = {'R', 'G', 'B', 'A'};
    for (std::size_t component = 0; component < 4; ++component) {
        const std::string name = std::string(clip) + "." + suffixes[component];
        for (const auto& channel : bank.channels) {
            if (channel.name == name) {
                result.rgba[component] = &channel;
                break;
            }
        }
    }
    return result;
}

std::uint32_t sample_sas_color_target(const SasColorChannels& channels,
                                      std::uint32_t original_rgba,
                                      float time) {
    std::uint32_t result = original_rgba;
    for (std::size_t component = 0; component < 4; ++component) {
        const auto* channel = channels.rgba[component];
        if (!channel) continue;
        // The native x87 conversion in 0x5a0fe0 uses the current rounding
        // mode, then writes the resulting low byte into the packed colour.
        const auto value = static_cast<std::uint8_t>(
            std::lrint(sample_sas_nonweighted_curve(*channel, time) * 255.0f));
        const auto shift = unsigned(component * 8);
        result = (result & ~(0xffu << shift)) | (std::uint32_t(value) << shift);
    }
    return result;
}

void SasColorSmoother::reset(std::uint32_t rgba) {
    current_ = rgba;
    delta_.fill(0.0f);
    has_timestamp_ = false;
}

float SasColorSmoother::update(std::uint32_t target_rgba,
                               std::uint64_t timestamp) {
    if (has_timestamp_ && timestamp == last_timestamp_) {
        float error = 0.0f;
        for (std::size_t component = 0; component < 4; ++component) {
            error += std::fabs(float(byte_at(current_, component)) -
                               float(byte_at(target_rgba, component))) *
                     (1.0f / 255.0f);
        }
        return error;
    }
    float error = 0.0f;
    std::uint32_t next_word = 0;
    for (std::size_t component = 0; component < 4; ++component) {
        const float old_byte = float(byte_at(current_, component));
        const float target_byte = float(byte_at(target_rgba, component));
        const float predicted = old_byte + delta_[component] * 0.4f;
        const float next = predicted + (target_byte - predicted) * 0.2f;
        delta_[component] = next - old_byte;
        const auto rounded = static_cast<std::uint8_t>(std::lrint(next));
        next_word |= std::uint32_t(rounded) << (component * 8);
        error += std::fabs(next - target_byte) * (1.0f / 255.0f);
    }
    current_ = next_word;
    last_timestamp_ = timestamp;
    has_timestamp_ = true;
    return error;
}

} // namespace hp6::presentation
