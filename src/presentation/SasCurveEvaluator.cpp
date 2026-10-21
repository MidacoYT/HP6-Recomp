#include "presentation/SasCurveEvaluator.h"

#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace hp6::presentation {
namespace {

float wrap_cycle(float time, float first, float last, bool before) {
    const float span = last - first;
    if (span == 0.0f) return first;
    // 0x575eb0 uses modf on an x87 double, then takes the absolute fraction.
    double integral = 0.0;
    const double fraction = std::fabs(std::modf(
        double(time - (before ? first : last)) / double(span), &integral));
    const float distance = float(fraction * double(span));
    return before ? last - distance : first + distance;
}

} // namespace

float sample_sas_nonweighted_curve(const SasAnimationChannelAsset& channel,
                                   float time) {
    if (channel.flags[0] != 0 || channel.flags[1] > 1) {
        throw std::runtime_error("unsupported SAS curve interpolation mode");
    }
    const auto& keys = channel.keys;
    if (keys.empty()) return 0.0f;
    const float first_time = keys.front().time;
    const float last_time = keys.back().time;
    if (time < first_time || time > last_time) {
        const bool before = time < first_time;
        const auto mode = channel.flags[before ? 2 : 3];
        if (mode == 0) return before ? keys.front().value : keys.back().value;
        if (mode != 2) {
            throw std::runtime_error("unsupported SAS curve boundary mode");
        }
        time = wrap_cycle(time, first_time, last_time, before);
    }
    // Native 0x575b70 returns the first value for this step flag throughout
    // the in-range interval, before searching for a key or segment.
    if (channel.flags[1] != 0) return keys.front().value;
    const auto right = std::lower_bound(keys.begin(), keys.end(), time,
        [](const SasAnimationKeyAsset& key, float sample_time) {
            return key.time < sample_time;
        });
    if (right == keys.end()) return keys.back().value;
    if (right->time == time || right == keys.begin()) return right->value;
    const auto& left = *(right - 1);
    if (left.tangents[2] == 0.0f && left.tangents[3] == 0.0f) {
        return left.value;
    }
    const float span = right->time - left.time;
    if (span <= 0.0f) {
        throw std::runtime_error("SAS curve keys are not ordered");
    }
    constexpr float third = 0.33333334f;
    const float left_handle_time = left.time + left.tangents[2] * third;
    const float left_handle_value = left.value + left.tangents[3] * third;
    const float right_handle_time = right->time - right->tangents[0] * third;
    const float right_handle_value = right->value - right->tangents[1] * third;
    constexpr float fallback_slope = 5729578.0f;
    const float start_width = left_handle_time - left.time;
    const float end_width = right->time - right_handle_time;
    const float start_slope = start_width != 0.0f ?
        (left_handle_value - left.value) / start_width : fallback_slope;
    const float end_slope = end_width != 0.0f ?
        (right->value - right_handle_value) / end_width : fallback_slope;
    const float delta_value = right->value - left.value;
    const float start_delta = start_slope * span;
    const float end_delta = end_slope * span;
    const float inverse_square = 1.0f / (span * span);
    const float cubic = (((end_delta + start_delta) - delta_value) -
                         delta_value) / span * inverse_square;
    const float quadratic = (((delta_value * 3.0f - start_delta) -
                             start_delta) - end_delta) * inverse_square;
    const float offset = time - left.time;
    return ((cubic * offset + quadratic) * offset + start_slope) * offset +
           left.value;
}

} // namespace hp6::presentation
