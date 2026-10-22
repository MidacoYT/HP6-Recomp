#include "platform/ServiceHeapLeafs.h"

#include <cstddef>

namespace hp6::platform {

std::uint8_t* heap_node_payload_005db440(void* node) {
    // 0x005DB440: add eax, 8; ret.
    return static_cast<std::uint8_t*>(node) + 8;
}

std::uint8_t* heap_relative_base_005db470(void* node) {
    // 0x005DB470: sub eax, [eax]; ret. The first word is a back offset.
    const auto offset = *static_cast<const std::uint32_t*>(node);
    return reinterpret_cast<std::uint8_t*>(
        reinterpret_cast<std::uintptr_t>(node) - offset);
}

std::uint8_t* heap_control_at_30_005db480(void* heap) {
    // 0x005DB480: add eax, 0x30; ret.
    return static_cast<std::uint8_t*>(heap) + 0x30;
}

std::uint32_t heap_header_flag_005db490(const void* heap) {
    // 0x005DB490: mov eax, [eax+4]; and eax, 1; ret.
    const auto* bytes = static_cast<const std::uint8_t*>(heap);
    return (*reinterpret_cast<const std::uint32_t*>(bytes + 4)) & 1u;
}

} // namespace hp6::platform
