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

// 0x0064c350 (cdecl). Direct3D 9 SetSamplerState (device vtable+0x114,
// index 69) with a per-(sampler,type) cache, same shape as
// set_texture_stage_state_cached above.
std::uint32_t set_sampler_state_cached(std::int32_t sampler, std::int32_t type,
                                       std::int32_t value);

// 0x0064c410 (cdecl). Direct3D 9 SetTexture (device vtable+0x104,
// index 65) with a per-stage cache.
std::uint32_t set_texture_cached(std::int32_t stage, void* texture);

// 0x0064c5c0 (cdecl). Direct3D 9 SetVertexDeclaration (device
// vtable+0x15c, index 87) with a cache.
std::uint32_t set_vertex_declaration_cached(void* declaration);

// 0x006524b0. Resets the two vertex-shader-constant cache words the
// exported x86 keeps alongside the vertex-shader cache.
void reset_vertex_shader_constant_cache();

// 0x0064c4e0 (cdecl). Direct3D 9 SetVertexShader (device vtable+0x170,
// index 92) with a cache; when `resolve` is set, the exported x86
// first resolves `handle` through 0x01003de0 (not traced — forwarded
// unchanged here, a documented no-op) and invalidates the
// vertex-shader-constant cache afterward.
std::uint32_t set_vertex_shader_cached(void* handle, bool resolve);

// 0x0064c570 (cdecl). Direct3D 9 SetPixelShader (device vtable+0x1ac,
// index 107) with a cache.
std::uint32_t set_pixel_shader_cached(void* handle);

// 0x0063b6e0 (fastcall). Direct3D 9 SetVertexShaderConstantF (device
// vtable+0x178, index 94): set_vertex_shader_constant_f(start_register,
// data, vector4_count). No cache (the exported x86 has none here).
void set_vertex_shader_constant_f(std::int32_t start_register, const void* data,
                                  std::int32_t vector4_count);

// 0x00652380 (cdecl). Direct3D 9 SetStreamSource (device vtable+400,
// index 100). `handle` resolves through one of two fixed 0x14-byte-stride
// tables (DAT_00a70890 for handle >= 0, DAT_00baa170 for handle < 0,
// i.e. the exported x86's own sign-bit split) to the real vertex buffer
// pointer at field+0x10; a no-op when handle is the 0xbad1bad1 "none"
// sentinel.
void bind_vertex_buffer_by_handle(std::uint32_t handle, std::uint32_t stride,
                                  std::uint32_t offset, std::uint32_t stream_number);

// 0x00652400 (cdecl). Direct3D 9 SetIndices (device vtable+0x1a0, index
// 104). `handle`'s sign bit selects between two fixed 0x14-byte-stride
// tables (DAT_0098c6a8 / DAT_009985a0) resolving to the real index
// buffer pointer at field+0x10.
void bind_index_buffer_by_handle(std::uint32_t handle);

// 0x0063b7a0 (cdecl). Looks up the pixel shader handle for the active
// render-region's mode (DAT_00998580, already written by
// set_active_render_region) in a fixed per-mode capability table
// (0xc4-byte stride, falling back to DAT_00cf1a3c's mode when the
// active mode is out of range or has no entry) -- the same table
// family FUN_0063b720/FUN_005e8e70 (not yet integrated) also read.
void* resolve_mode_pixel_shader_handle();

// 0x010043b0 (cdecl). Binds the mode's pixel shader (resolve_mode_
// pixel_shader_handle + set_pixel_shader_cached, both above) only when
// `handle` differs from the cached one or `force` is set; counts the
// rebind in DAT_00b170c4.
void bind_pixel_shader_for_mode(std::int32_t handle, bool force);

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
