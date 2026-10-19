#pragma once
#include <cstdint>

namespace hp6::render {

// 0x00648c00 (cdecl). Locks the shared dynamic vertex buffer
// (DAT_00c75a7c, already created by create_dynamic_vertex_buffer),
// interleaves up to `vertex_count` (< 0x400) vertices from `data` into
// it according to `flags` (bit 0: 4 position floats per vertex from
// `data`; bit 2: 1 color dword per vertex from `data`, else a fixed
// opaque white 0xffffffff; bit 3: 2 UV floats per vertex from `data`,
// else (0,0)) — each vertex always occupies 7 dwords (28 bytes) in the
// buffer regardless of which fields came from `data` — then unlocks,
// binds it via SetStreamSource(0, ..., 0, 28) and the cached vertex
// declaration/shader (DAT_00cf1930/DAT_00cf192c, both still untraced
// fixed globals — forwarded as opaque handles, not fabricated), and
// issues the real Direct3D 9 draw call: DrawPrimitive(D3DPT_
// TRIANGLESTRIP, 0, vertex_count - 2) (device vtable+0x144, index 81).
// Used by render_fade_overlay_quad below to draw a 4-vertex screen
// quad (2 triangles).
void submit_screen_quad(const float* data, std::uint32_t flags, std::int32_t vertex_count);

// 0x0064a9e0 (stdcall). The real "arms elsewhere" site
// render_fade_overlay_quad's own doc comment below refers to: a no-op
// (returns false) when hp6::render::handle_device_lost reports the
// device isn't ready to render this tick (device lost, mid-reset, or
// TestCooperativeLevel still failing); otherwise refreshes the cached
// available texture memory (redundant with handle_device_lost's own
// internal call, reproduced anyway for fidelity), issues the real
// Direct3D 9 BeginScene (device vtable+0xa4, index 41), increments
// DAT_00cf19e4 (arming render_fade_overlay_quad's own one-shot
// countdown by one), and reports whether BeginScene itself succeeded
// (HRESULT == 0). Reached, in the exported x86, only through
// FUN_01002c80 (a large debug-only scratch-buffer initializer, itself
// reached only via the same confirmed-dead debug-overlay chain
// render_fade_overlay_quad's doc comment describes) — NOT YET
// INTEGRATED, since its own value is never observably used by anything
// this project's tests depend on, only its side effect on
// DAT_00cf19e4 matters here.
bool try_begin_debug_overlay_scene();

// 0x0064abd0 (cdecl). Composites the render-target-as-texture
// (DAT_00cf1958, when not disabled) as a full-screen, `color`-tinted
// quad (built directly in this function: 4 vertices, z/rhw/UV corners
// from fixed constants DAT_00831c2c/DAT_007db234 — both still
// untraced opaque floats, forwarded faithfully rather than guessed —
// and screen-space X bounds computed from the active display
// width/height, DAT_0080a8b0 and DAT_0080a8d0, adjusted when
// DAT_00cf1994/FilterFlip is set), via submit_screen_quad above, then
// unconditionally calls EndScene (device vtable+0xa8) and Present
// (device vtable+0x44). Guarded by a one-shot countdown
// (DAT_00cf19e4/DAT_00cf19e0), armed by try_begin_debug_overlay_scene
// above. Reached, in the exported x86, only from the debug build's
// overlay-rendering chain (FUN_005dd020 -> thunk_FUN_010120d0 ->
// FUN_00648630 -> this function) which hp6::presentation::
// is_debug_build_overlay_enabled already confirms is hardcoded false
// (dead) in the shipped FINAL build — this function itself is
// faithfully reconstructed from real disassembly regardless, since a
// test can still exercise it directly against a fake device.
void render_fade_overlay_quad(std::uint32_t color);

} // namespace hp6::render
