#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace hp6::platform {

// Three observed SystemParametersInfoA buffers: 8, 8 and 24 bytes on x86.
struct SystemParameterBuffers {
    std::array<std::byte, 8> first;
    std::array<std::byte, 8> second;
    std::array<std::byte, 24> third;
};

using SystemParameterCall = bool (*)(std::uint32_t action, std::uint32_t size,
                                     void* data, std::uint32_t options);

void apply_system_parameters(bool restore, SystemParameterBuffers& saved,
                             SystemParameterCall call); // 0x005d9210
std::int32_t startup_resource_size(std::int32_t a,
                                   std::int32_t b); // 0x006dadb9

}
