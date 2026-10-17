#include "platform/StartupSupport.h"
#include <cstring>

namespace hp6::platform {
namespace {

std::uint32_t flags(const std::byte* data) {
    std::uint32_t value;
    std::memcpy(&value, data + 4, 4);
    return value;
}

void clear_mode_bits(std::byte* data) {
    const auto value = flags(data) & 0xfffffff3u;
    std::memcpy(data + 4, &value, 4);
}

}

void apply_system_parameters(bool restore, SystemParameterBuffers& saved,
                             SystemParameterCall call) {
    if (restore) {
        call(0x3b, 8, saved.first.data(), 0);
        call(0x35, 8, saved.second.data(), 0);
        call(0x33, 0x18, saved.third.data(), 0);
        return;
    }
    auto first = saved.first;
    if ((flags(saved.first.data()) & 1u) == 0) {
        clear_mode_bits(first.data());
        call(0x3b, 8, first.data(), 0);
    }
    auto second = saved.second;
    if ((flags(saved.second.data()) & 1u) == 0) {
        clear_mode_bits(second.data());
        call(0x35, 8, second.data(), 0);
    }
    auto third = saved.third;
    if ((flags(saved.third.data()) & 1u) == 0) {
        clear_mode_bits(third.data());
        call(0x33, 0x18, third.data(), 0);
    }
}

std::int32_t startup_resource_size(std::int32_t a, std::int32_t b) {
    return static_cast<std::int32_t>(
        static_cast<std::uint32_t>(b) * 0x20u +
        static_cast<std::uint32_t>(a) * 0x120u + 0x58u);
}

}
