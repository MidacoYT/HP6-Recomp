#pragma once

#include <cstdint>

#if defined(_MSC_VER) && defined(_M_IX86)
#define HP6_PATH_FASTCALL __fastcall
#else
#define HP6_PATH_FASTCALL
#endif

namespace hp6::audio {

// PATH.lib virtual GetVolume at 006c1d20. The base vtable at 00718210
// places it at +0x2c; SetVolume (006c1d40) calls that slot.
std::uint32_t HP6_PATH_FASTCALL path_get_volume(const std::uint8_t* path_object);

// PATH.lib 006be290. The +0x10 field is written by the dry-level setters
// 006c2980 and 006c3be0, which call 006c1e00 (PF_dr / DRYLEVEL).
std::uint32_t HP6_PATH_FASTCALL path_get_dry_level(const std::uint8_t* path_object);

} // namespace hp6::audio
