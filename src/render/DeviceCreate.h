#pragma once
#include <cstdint>

namespace hp6::render {

// 0x00649140 ("BuildAdapterModeBlock" in the independent IDA knowledge base
// at D:\HP\knowledge\hp6, cross-checked against this project's own Ghidra
// export). Lazily builds the per-adapter Direct3D 9 mode table at
// DAT_00cf19d0 (0x584 bytes per adapter: a D3DADAPTER_IDENTIFIER9 at +0, a
// D3DCAPS9 — queried with D3DDEVTYPE_HAL — at +0x454, and a filtered, sorted
// array of {Width,Height,RefreshRate,Format} entries at +0x450, with its
// count at +0x44c) the first time it runs; every later call is a no-op
// (returns 0 immediately) because the guard is DAT_00cf19d0 itself. For each
// adapter whose D3DCAPS9.DeviceType is D3DDEVTYPE_HAL (offset +0x454 == 1),
// walks the three back-buffer formats a HAL device can present
// (D3DFMT_R5G6B5=0x16, D3DFMT_X8R8G8B8=0x17, D3DFMT_A8R8G8B8=0x18) through
// GetAdapterModeCount/EnumAdapterModes, keeps only modes at least 320x200,
// deduplicates by (Width,Height,RefreshRate), and sorts the per-adapter list
// with the qsort comparator at 0x006490b0 (refresh rate descending, then
// width ascending, then height ascending).
std::int32_t enumerate_display_modes();

// 0x004d6420. DAT_008663c8 = DAT_008663c4 — a plain settings-field copy;
// neither global's producer/consumer beyond this project's current render
// module is resolved yet.
void copy_brightness_setting();

// 0x0063b560. Recomputes the quality-tier flag DAT_008663d5 from the
// escalation counter DAT_00cf19bc and the cached available-texture-memory
// value DAT_00cf1964 (the same fields read by
// hp6::render::adjust_texture_quality_for_memory in DeviceReset.cpp).
void update_quality_tier_flag();

// 0x0063b880 (thunk of 0x01000c60). The exported x86 passes its single
// argument through EAX rather than the stack at both of this project's call
// sites (confirmed in the raw assembly of create_d3d9_device); made explicit
// here as a normal parameter. value==1 clears DAT_00866364; value==2 sets it
// to 2; any other value is a no-op.
void apply_multisample_quality(std::int32_t value);

// 0x0064b560. Zeroes the four consecutive 32-bit words starting at
// DAT_00bb09b4; exact purpose of that block is not cross-referenced in this
// batch.
void clear_scratch_light_block();

// 0x01003300 (reached from create_d3d9_device via the thunk at 0x0064bb90).
// Fills two fixed-stride (0x80 bytes), 8-entry default-value tables — one
// starting at DAT_00b108a4 for per-stage sampler states, one starting at
// DAT_009968dc for per-stage texture-stage states — with this module's
// hardcoded defaults, then calls apply_default_sampler_and_stage_states.
void init_default_sampler_and_stage_state_table();

// 0x0064bd90. Applies the two 8-entry default-value tables filled by
// init_default_sampler_and_stage_state_table to the device (DAT_00cf1944)
// via SetSamplerState (vtable+0x114, index 69) and SetTextureStageState
// (vtable+0x10c, index 67), one full 8-stage pass per table.
void apply_default_sampler_and_stage_states();

// 0x01003010 (reached from create_d3d9_device via the thunk at 0x0064b970).
// Fills a large block of fixed default Direct3D 9 render-state values
// (DAT_00c751a4..DAT_00c75440) with this module's hardcoded defaults, then
// calls apply_default_render_states.
void init_default_render_state_table();

// 0x0064b5c0. Applies the render-state block filled by
// init_default_render_state_table to the device (DAT_00cf1944) via
// SetRenderState (vtable+0xe4, index 57) directly — not through the cache
// in RenderStateCache.cpp, matching the exported x86 exactly.
void apply_default_render_states();

// 0x0064b5a0 (thunk of 0x01003550). Calls, in order: reset_hook_placeholder,
// clear_scratch_light_block, init_default_sampler_and_stage_state_table,
// init_default_render_state_table.
void apply_secondary_render_state_defaults();

// 0x0064c8c0 ("BuildResolutionOptionList_HardCap" in the independent IDA
// knowledge base). NOT YET INTEGRATED: this is create_d3d9_device's
// alternate branch for an explicit/overridden resolution list, reached only
// when DAT_00cf1950 is non-zero. Nothing integrated into this project so far
// ever sets DAT_00cf1950 to a non-zero value, so this branch is dead code in
// the current reconstruction; tracing it fully (thunk_FUN_01002f70,
// thunk_FUN_01003650, FUN_005da610, FUN_0064c800, FUN_00404660,
// FUN_00457530, and several more unresolved globals including the
// already-flagged-unknown DAT_007db234) is deferred to a future batch. The
// definition in DeviceCreate.cpp exists only so dependents link; it aborts
// loudly if ever actually reached, rather than silently faking the real
// resolution-list logic.
void build_resolution_option_list_hard_cap();

// 0x005da4b0. NOT YET INTEGRATED: create_d3d9_device's fatal-error path,
// reached only when Direct3DCreate9 or CreateDevice fails. In the exported
// x86 it formats a diagnostic string, shows a MessageBoxW, and always ends
// by calling TerminateProcess — so on a correctly functioning machine
// (compatible GPU/driver) this project's reconstruction of create_d3d9_device
// never reaches it. The definition in DeviceCreate.cpp exists only so
// dependents link; it aborts loudly (matching the original's own
// process-terminating outcome, without reconstructing the diagnostic
// message/MessageBoxW chain) rather than silently faking success.
void fatal_display_error();

// 0x006498e0 ("CreateD3DDeviceAndSwapchains" in the independent IDA
// knowledge base, cross-checked against this project's own Ghidra export —
// both agree on every call site). Called once, from FUN_005dca10
// (0x005dca10, itself called directly from WinMain at 0x005d9300) as
// create_d3d9_device(client_width, client_height, 0, 0, 6). This is the
// game's actual IDirect3D9::CreateDevice call site, previously unresolved
// (see architecture/startup_path.md): creates the IDirect3D9 interface via
// Direct3DCreate9, builds the adapter/mode table (enumerate_display_modes),
// picks the first HAL-capable adapter, resolves a compatible back-buffer
// format via CheckDeviceType (falling back between the 16-/32-bit pairs
// 0x17/0x16 and 0x16/0x15 on failure), fills a D3DPRESENT_PARAMETERS block
// at DAT_00c75538, calls CreateDevice (retrying once at a safer behavior
// flag on failure), then clears/presents the fresh device, creates a
// depth-stencil surface and (unless DAT_00cf1998 disables it) a matching
// render-target texture and a depth-stencil-as-texture pair, binds them,
// and clears/presents once more before handing off to
// create_dynamic_vertex_buffer (0x00648e10, already integrated in
// DeviceReset.cpp). Returns 0 on success, else an HRESULT-shaped failure
// code (0x80004005, E_FAIL) — though every failure branch observed also
// calls fatal_display_error or build_resolution_option_list_hard_cap first.
std::int32_t create_d3d9_device(std::uint32_t width, std::uint32_t height,
                                 std::uint32_t param_3, std::uint32_t param_4,
                                 std::uint32_t param_5);

// 0x005dca10. Called directly from WinMain (0x005d9300) with the client
// area's width/height, right after window creation. Resets the frame-pump
// state (hp6::runtime::reset_frame_pump_state), initializes the render
// framework's main heap (hp6::render::initialize_render_framework_heap, on
// the fixed global object at 0x00d71250), creates the real D3D9 device
// (create_d3d9_device(width, height, 0, 0, 6)) — showing the cursor again
// if that fails with E_FAIL, matching the exported x86's one special-cased
// branch — then invokes vtable slot 0 of the engine-service object at
// DAT_00d6af14 with (0, 0). That object is read (never written) by five
// functions across this binary's life cycle, including the engine
// bootstrap FUN_005dcdf0 (Engine_InitCoreSystemsAndMessages, not yet
// reconstructed) and the two post-message-loop cleanup functions
// (FUN_005d7250/FUN_005dced0) already noted in architecture/
// startup_path.md; its single write site is outside any function Ghidra
// could name (likely a static C++ initializer), so its concrete type is
// not yet identified. NOT YET INTEGRATED here: the vtable call itself is a
// documented, abort()-ing placeholder (see DeviceCreate.cpp), since
// resolving the real object requires reconstructing FUN_005dcdf0 first.
std::int32_t create_and_configure_render_device(std::uint32_t width, std::uint32_t height);

// 0x005dced0 and 0x005d7250. Called once each from WinMain, right after
// run_message_loop returns, before hp6::render::shutdown_render_device.
// Both read the same still-unidentified DAT_00d6af14 engine-service
// object as create_and_configure_render_device above (vtable slot +0xc
// for the first, +4 for the second), alongside a dozen other untraced
// globals each (an FMV-playback object, several named heap handles, a
// second lazy service-holder duplicate). Documented no-op placeholders,
// same status and same reasoning as invoke_engine_service_hook.
void run_post_message_loop_cleanup_primary(std::int32_t param);
void run_post_message_loop_cleanup_secondary();

} // namespace hp6::render
