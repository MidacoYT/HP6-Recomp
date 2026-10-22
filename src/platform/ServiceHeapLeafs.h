#pragma once

#include <cstdint>

namespace hp6::platform {

// The caller at 0x005DD6B0 supplies these values in EAX in the x86 image.
std::uint8_t* heap_node_payload_005db440(void* node);
std::uint8_t* heap_relative_base_005db470(void* node);
std::uint8_t* heap_control_at_30_005db480(void* heap);
std::uint32_t heap_header_flag_005db490(const void* heap);

} // namespace hp6::platform
