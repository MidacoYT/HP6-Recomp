#pragma once
#include <cstdint>

namespace hp6::render {

// 0x0064c460. Direct3D 9 SetRenderTarget (device vtable+0x94, index 37)
// and SetDepthStencilSurface (vtable+0x9c, index 39), skipped when both
// `render_target`/`depth_stencil` already match the cached values at
// DAT_00bb0a40/DAT_00ba14f8 (the same cache RenderTargetSetup.cpp's
// prepare_render_target_texture writes). Returns the SetRenderTarget
// HRESULT, or 0 when skipped.
std::int32_t set_render_targets_cached(void* render_target, void* depth_stencil);

// 0x0064a6d0. Releases the shared dynamic vertex buffer
// (hp6::render::create_dynamic_vertex_buffer's own DAT_00c75a7c) and
// every render-target/depth-stencil resource
// hp6::render::prepare_render_target_texture created (DAT_00cf1958,
// DAT_00cf1960, DAT_00cf1954, DAT_00cf195c), via each object's own
// Release (vtable+8) — including, when a render-to-texture is active
// (DAT_00cf1958 neither null nor the 0xbacb0ffe disabled sentinel), one
// extra GetBackBuffer/Release pair to drop the swap chain's own
// reference to the surface being released. Called from the device-lost
// handler (create_d3d9_device_after_reset below) before Reset(), so
// every one of these is recreated against the fresh device afterward.
void release_dynamic_render_resources();

// 0x0064aa60. The classic Direct3D 9 device-lost/reset handler, called
// once per message-loop iteration before PeekMessageA (see
// hp6::presentation — the Win32 message loop itself,
// architecture/startup_path.md). A no-op (returns success) when no
// device exists yet. Otherwise refreshes the cached available texture
// memory (cache_available_texture_memory), then TestCooperativeLevel's
// (device vtable+0xc) result decides what happens next: a clean device
// falls straight through to prepare_render_target_texture;
// D3DERR_DEVICELOST sleeps 50ms and retries next iteration;
// D3DERR_DEVICENOTRESET (or any case where DAT_00cf19dc, a forced-reset
// flag, is set) releases every dynamic render resource and calls
// Reset() (device vtable+0x40) with the present parameters this
// project's create_d3d9_device already fills in — any other HRESULT
// from TestCooperativeLevel is a fatal diagnostic
// (hp6::platform::format_diagnostic_message, matching the exported
// x86's own un-recoverable error strings exactly), and a failed Reset()
// either retries (D3DERR_DEVICELOST) or is fatal the same way. On a
// successful Reset(), rebuilds the render target through
// prepare_render_target_texture exactly like the clean-device path.
std::uint32_t handle_device_lost();

// 0x00648e70. Called once from WinMain, right after run_message_loop
// returns, tearing down everything the render device chain owns: an
// untraced auxiliary object (DAT_00cf19d8), the shared dynamic vertex
// buffer, every allocated per-adapter mode-table sub-buffer (the
// records create_d3d9_device's enumerate_display_modes malloc'd into
// DAT_00cf19d0, at +0x450 within each 0x584-byte stride, counted by
// DAT_00cf19c8), the render-target/depth-stencil resources
// release_dynamic_render_resources also knows about (in a slightly
// different order — this is a distinct address, reconstructed
// directly from its own disassembly, not a thin wrapper around that
// function), the per-adapter table itself (DAT_00cf19d0), and finally
// the device and the IDirect3D9 object (DAT_00cf1944/DAT_00cf194c),
// each asserting its refcount dropped to zero on release (matching the
// exported x86's own fatal "Device refs %d\n"/"D3D refs %d\n"
// diagnostics via hp6::platform::format_diagnostic_message, which is
// not actually noreturn — see Diagnostics.h).
void shutdown_render_device();

} // namespace hp6::render
