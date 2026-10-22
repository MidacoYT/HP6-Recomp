#include "platform/Sync.h"

namespace hp6::platform {

// Unprotected hp6.exe 006d8f90: 8b4120c3 (MOV EAX,[ECX+20h]; RET).
std::uint32_t HP6_SYNC_FASTCALL lock_acquisition_depth(const std::uint8_t* lock) {
    return *reinterpret_cast<const std::uint32_t*>(lock + 0x20);
}

} // namespace hp6::platform
