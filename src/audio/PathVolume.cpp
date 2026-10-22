#include "audio/PathVolume.h"

namespace hp6::audio {

// Unprotected hp6.exe 006c1d20:
// 558bec51c745fccccccccc894dfc8b45fc8b40148be55dc3
std::uint32_t HP6_PATH_FASTCALL path_get_volume(const std::uint8_t* path_object) {
    return *reinterpret_cast<const std::uint32_t*>(path_object + 0x14);
}

// Unprotected hp6.exe 006be290:
// 558bec51c745fccccccccc894dfc8b45fc8b40108be55dc3
std::uint32_t HP6_PATH_FASTCALL path_get_dry_level(const std::uint8_t* path_object) {
    return *reinterpret_cast<const std::uint32_t*>(path_object + 0x10);
}

} // namespace hp6::audio
