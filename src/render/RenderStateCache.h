#pragma once
#include <cstdint>

namespace hp6::render {

// 0x0064c300 (cdecl). Direct3D 9 SetRenderState (device vtable+0xe4,
// index 57) with a per-state cache at DAT_00c75188[state] that skips the
// call when `value` is unchanged. Returns 0 when skipped, else the
// HRESULT from SetRenderState.
std::uint32_t set_render_state_cached(std::int32_t state, std::int32_t value);

// 0x0064c3b0 (cdecl). Direct3D 9 SetTextureStageState (device
// vtable+0x10c, index 67) with a per-(stage,type) cache at
// DAT_009968d8[stage * 0x20 + type] that skips the call when `value` is
// unchanged. Returns 0 when skipped, else the HRESULT.
std::uint32_t set_texture_stage_state_cached(std::int32_t stage, std::int32_t type,
                                             std::int32_t value);

// 0x010021f0 (reached only through the thunk at 0x006406a0; `settings` is
// passed in ESI in the exported x86, not a stack argument — a calling
// convention this project has not needed before). Applies 12 render
// states (settings[1..12], states 0x13/0x14/0x1b/0x80..0x87/0xab) and 6
// stage-0 texture stage states (settings[0xd..0x12], types 1..6) through
// the caches above, then copies settings[0..0x12] verbatim into the
// fixed globals DAT_0086527c and DAT_0088137x/DAT_008813xx (19 words from
// DAT_00881374) for later reference.
void apply_render_state_profile(const std::uint32_t* settings);

// 0x00640fb0. Fills a 19-word scratch profile (the DAT_0088137x/
// DAT_008813xx globals, reached as one block at &DAT_00881370) with this
// module's hardcoded default render/texture-stage states — most values
// are fixed, a block of 14 of them changes when the active display
// mode's capability byte at mode+0x474 has bit 0x80 clear, or the pixel
// format (DAT_00cf19a8) is 0x10 — then calls apply_render_state_profile
// on that same buffer and overwrites its first word's effect with the
// fixed sentinels DAT_0086527c=-8 and DAT_00865280=-7.
void initialize_default_render_state();

} // namespace hp6::render
