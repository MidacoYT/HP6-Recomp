// Local behavior checks for FUN_0064aa60 (the classic Direct3D 9
// device-lost/reset handler, called once per Win32 message-loop
// iteration — see architecture/startup_path.md) and its two helpers,
// FUN_0064a6d0 and FUN_0064c460. Game-space symbols and a fake
// IDirect3DDevice9-shaped vtable get test-local definitions, the way
// the other render smoke tests already do.
#include "render/DeviceLostHandler.h"
#include "render/DeviceReset.h"
#include "render/RenderStateCache.h"
#include "render/RenderTargetSetup.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#define HP6_STDCALL __stdcall
#else
#define HP6_STDCALL
#endif

// ---- DeviceLostHandler.cpp's own extern dependencies ----------------------
void* DAT_00cf1944 = nullptr;
std::uint8_t DAT_00cf19dc = 0;
std::uint8_t DAT_00cf18c7 = 0;
std::uint32_t DAT_00c75538[14] = {};
std::int32_t DAT_00cf1938 = 0;
std::uint8_t DAT_00cf19df = 0;

// ---- pulled in transitively through prepare_render_target_texture /
// create_dynamic_vertex_buffer / cache_available_texture_memory / the
// default render-state profile (same set tests/render_state_cache_smoke.cpp
// already establishes for the same reason) -------------------------------
std::int32_t DAT_00c75188[256] = {};
std::int32_t DAT_009968d8[256 * 0x20] = {};
std::uint32_t DAT_00cf18c4 = 0, DAT_00cf18c8 = 0;
std::int32_t DAT_00cf18cc = 0, DAT_00cf18e0 = 0;
std::uint32_t DAT_00881370[19] = {};
std::uint32_t DAT_008813bc = 0, DAT_008813c0 = 0, DAT_008813c4 = 0;
std::uint32_t DAT_00f82c40 = 0, DAT_00f82c44 = 0, DAT_00f82c48 = 0, DAT_00f82c8c = 0;
std::uint32_t DAT_00f82c90 = 0, DAT_00f82c94 = 0, DAT_00f82cd8 = 0, DAT_00f82cdc = 0;
std::uint32_t DAT_00f82ce0 = 0, DAT_00f82d24 = 0, DAT_00f82d28 = 0, DAT_00f82d2c = 0;
std::uint32_t DAT_00f82d70 = 0, DAT_00f82d74 = 0, DAT_00f82d78 = 0, DAT_00f82dbc = 0;
std::uint32_t DAT_00f82dc0 = 0, DAT_00f82dc4 = 0, DAT_00f82e08 = 0, DAT_00f82e0c = 0;
std::uint32_t DAT_00f82e10 = 0, DAT_00f82e54 = 0, DAT_00f82e58 = 0, DAT_00f82e5c = 0;
std::uint32_t DAT_00f82ea0 = 0, DAT_00f82ea4 = 0, DAT_00f82ea8 = 0, DAT_00f82ecc = 0;
std::uint32_t DAT_00f82eec = 0, DAT_00f82ef0 = 0, DAT_00f82ef4 = 0, DAT_00f82f38 = 0;
std::uint32_t DAT_00f82f3c = 0, DAT_00f82f40 = 0, DAT_00f82f84 = 0, DAT_00f82f88 = 0;
std::uint32_t DAT_00f82f8c = 0, DAT_00f82fd0 = 0, DAT_00f82fd4 = 0, DAT_00f82fd8 = 0;
std::uint32_t DAT_00f8301c = 0, DAT_00f83020 = 0, DAT_00f83024 = 0, DAT_00f83068 = 0;
std::uint32_t DAT_00f8306c = 0, DAT_00f83070 = 0, DAT_00f830b4 = 0, DAT_00f830b8 = 0;
std::uint32_t DAT_00f830bc = 0, DAT_00f83100 = 0, DAT_00f83104 = 0, DAT_00f83108 = 0;
std::uint32_t DAT_00f8314c = 0, DAT_00f83150 = 0, DAT_00f83154 = 0, DAT_00f83198 = 0;
std::uint32_t DAT_00f8319c = 0, DAT_00f831a0 = 0, DAT_00f831e4 = 0, DAT_00f831e8 = 0;
std::uint32_t DAT_00f831ec = 0, DAT_00f83230 = 0, DAT_00f83234 = 0, DAT_00f83238 = 0;
std::uint32_t DAT_00f8327c = 0, DAT_00f83280 = 0, DAT_00f83284 = 0, DAT_00f832c8 = 0;
std::uint32_t DAT_00f832cc = 0, DAT_00f832d0 = 0, DAT_00f83314 = 0, DAT_00f83318 = 0;
std::uint32_t DAT_00f8331c = 0, DAT_00f83340 = 0, DAT_00f83360 = 0, DAT_00f83364 = 0;
std::uint32_t DAT_00f83368 = 0, DAT_00f8338c = 0, DAT_00f833ac = 0, DAT_00f833b0 = 0;
std::uint32_t DAT_00f833b4 = 0, DAT_00f833d8 = 0, DAT_00f83494 = 0, DAT_00f83498 = 0;
std::uint32_t DAT_00f834a0 = 0, DAT_00f834a4 = 0, DAT_00f834ac = 0, DAT_00f834b0 = 0;
std::uint32_t DAT_00f834b8 = 0, DAT_00f834bc = 0, DAT_00f834c4 = 0, DAT_00f834c8 = 0;
std::uint32_t DAT_00f834d0 = 0, DAT_00f834d4 = 0, DAT_00f834dc = 0, DAT_00f834e0 = 0;
std::uint32_t DAT_00f834e8 = 0, DAT_00f834ec = 0, DAT_00f834f4 = 0, DAT_00f834f8 = 0;
std::uint32_t DAT_00f83500 = 0, DAT_00f83504 = 0, DAT_00f8350c = 0, DAT_00f83510 = 0;
std::int32_t DAT_008663d8 = 0;
std::int32_t DAT_00cf19d0 = 0;
std::uint32_t DAT_00cf19a8 = 0;
std::int32_t DAT_0086527c = 0, DAT_00865280 = 0;
std::uint8_t DAT_00865286 = 0;
std::uint32_t DAT_00cf19d4 = 0;
std::int32_t DAT_00b108a0[256] = {};
std::uint32_t DAT_00bb09b4[16] = {};
void* DAT_00cf19c4 = nullptr;
std::uint32_t DAT_008663cc = 0;
std::uint32_t DAT_008663d0 = 0;
std::uint32_t DAT_0096b484 = 0;
std::uint32_t DAT_00bac98c = 0;
std::uint8_t DAT_00a70890[0x14] = {};
std::uint8_t DAT_00baa170[0x14] = {};
std::uint8_t DAT_0098c6a8[0x14] = {};
std::uint8_t DAT_009985a0[0x14] = {};
std::int32_t DAT_00998580 = 0;
std::int32_t DAT_00c754cc = 0;
std::uint8_t DAT_00f38b28[0xc4] = {};
std::int32_t DAT_00cf1a3c = 0;
std::int32_t DAT_00cf1a48 = 0;
std::uint32_t DAT_00b170c4 = 0;
void* DAT_00c75a7c = nullptr;
struct ShaderSlot { std::uint32_t bytecode; std::uint32_t shader; };
ShaderSlot DAT_00a68820[4] = {};
std::uint32_t DAT_00cf1a30 = 0;
std::uint32_t DAT_00cf1a38 = 0;
std::uint8_t DAT_00f38b10[0x600 * 0xc4] = {};
std::int32_t DAT_00cf19bc = 0;
std::uint32_t DAT_00cf1964 = 0;
std::uint8_t DAT_008663d5 = 0;
std::int32_t DAT_008928c0[256] = {};
std::int32_t DAT_008663b8 = 0;
std::uint32_t DAT_008663dc = 0;
std::int32_t DAT_00cf1974 = 0;
void* DAT_00cf1948 = nullptr;
void* DAT_00cf1954 = nullptr;
void* DAT_00cf1958 = nullptr;
void* DAT_00cf195c = nullptr;
void* DAT_00cf1960 = nullptr;
void* DAT_00cf19d8 = nullptr; // shutdown_render_device's own untraced auxiliary object
void* DAT_00cf194c = nullptr; // IDirect3D9* (also in DeviceCreate.cpp)
std::uint32_t DAT_00cf19c8 = 0; // adapter count (also in DeviceCreate.cpp)
void* DAT_00bb0a40 = nullptr;
void* DAT_00ba14f8 = nullptr;
std::int32_t DAT_00cf1998 = 0;
std::uint8_t DAT_008663d4 = 0;
std::uint8_t DAT_00865287 = 0;
std::uint32_t DAT_008663b0 = 0;
std::uint32_t DAT_008663b4 = 0;
std::uint32_t DAT_00b170b8 = 0;

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

int release_calls = 0;
void HP6_STDCALL fake_release(void*) { ++release_calls; }

int test_level_calls = 0;
std::int32_t test_level_result = 0;
std::int32_t HP6_STDCALL fake_test_cooperative_level(void*) {
    ++test_level_calls;
    return test_level_result;
}

int reset_calls = 0;
std::int32_t reset_result = 0;
void* last_reset_params = nullptr;
std::int32_t HP6_STDCALL fake_reset(void* device, void* params) {
    (void)device;
    ++reset_calls;
    last_reset_params = params;
    return reset_result;
}

int get_desc_calls = 0;
std::int32_t HP6_STDCALL fake_get_desc(void*, void* out) {
    ++get_desc_calls;
    std::memset(out, 0, 0x20);
    return 0;
}
void* surface_vtable[16] = {};
struct FakeSurface { void* vtable = surface_vtable; };
FakeSurface fake_back_buffer_surface;

int get_back_buffer_calls = 0;
std::int32_t HP6_STDCALL fake_get_back_buffer(void*, std::uint32_t, std::uint32_t, void* out) {
    ++get_back_buffer_calls;
    *reinterpret_cast<void**>(out) = &fake_back_buffer_surface;
    return 0;
}

int get_available_texture_mem_calls = 0;
std::uint32_t HP6_STDCALL fake_get_available_texture_mem(void*) {
    ++get_available_texture_mem_calls;
    return 0;
}

std::int32_t HP6_STDCALL fake_set_render_state(void*, std::uint32_t, std::uint32_t) { return 0; }
std::int32_t HP6_STDCALL fake_set_texture_stage_state(void*, std::uint32_t, std::uint32_t,
                                                       std::uint32_t) {
    return 0;
}

int set_render_target_calls = 0;
std::int32_t HP6_STDCALL fake_set_render_target(void*, std::uint32_t, void*) {
    ++set_render_target_calls;
    return 0;
}
int set_depth_stencil_calls = 0;
std::int32_t HP6_STDCALL fake_set_depth_stencil(void*, void*) {
    ++set_depth_stencil_calls;
    return 0;
}
int create_depth_stencil_calls = 0;
std::int32_t HP6_STDCALL fake_create_depth_stencil(void*, std::uint32_t, std::uint32_t,
                                                   std::uint32_t, std::uint32_t, std::uint32_t,
                                                   std::uint32_t, void* out, std::uint32_t) {
    ++create_depth_stencil_calls;
    static void* depth_stencil_vtable[16] = {};
    static bool vtable_ready = false;
    if (!vtable_ready) {
        depth_stencil_vtable[8 / sizeof(void*)] = reinterpret_cast<void*>(&fake_release);
        vtable_ready = true;
    }
    static struct { void* vtable = depth_stencil_vtable; } fake_depth_stencil_surface;
    *reinterpret_cast<void**>(out) = &fake_depth_stencil_surface;
    return 0;
}
void* vertex_buffer_vtable[4] = {};
struct FakeVertexBuffer { void* vtable = vertex_buffer_vtable; };
FakeVertexBuffer fake_vertex_buffer;

int create_vb_calls = 0;
std::int32_t HP6_STDCALL fake_create_vertex_buffer(void*, std::uint32_t, std::uint32_t,
                                                   std::uint32_t, std::uint32_t, void* out,
                                                   std::uint32_t) {
    ++create_vb_calls;
    *reinterpret_cast<void**>(out) = &fake_vertex_buffer;
    return 0;
}

void* device_vtable[0x120 / sizeof(void*) + 1] = {};
struct FakeDevice { void* vtable = device_vtable; };
FakeDevice fake_device;

void* swap_chain_vtable[8] = {};
struct FakeSwapChain { void* vtable = swap_chain_vtable; };
FakeSwapChain fake_swap_chain;

} // namespace

int main() {
    using namespace hp6::render;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    device_vtable[0xc / sizeof(void*)] = reinterpret_cast<void*>(&fake_test_cooperative_level);
    device_vtable[0x40 / sizeof(void*)] = reinterpret_cast<void*>(&fake_reset);
    device_vtable[8 / sizeof(void*)] = reinterpret_cast<void*>(&fake_release);
    device_vtable[0x10 / sizeof(void*)] =
        reinterpret_cast<void*>(&fake_get_available_texture_mem);
    device_vtable[0x94 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_render_target);
    device_vtable[0x9c / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_depth_stencil);
    device_vtable[0x74 / sizeof(void*)] = reinterpret_cast<void*>(&fake_create_depth_stencil);
    device_vtable[0x68 / sizeof(void*)] = reinterpret_cast<void*>(&fake_create_vertex_buffer);
    vertex_buffer_vtable[8 / sizeof(void*)] = reinterpret_cast<void*>(&fake_release);
    device_vtable[0xe4 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_render_state);
    device_vtable[0x10c / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_texture_stage_state);
    swap_chain_vtable[8 / sizeof(void*)] = reinterpret_cast<void*>(&fake_release);
    swap_chain_vtable[0x14 / sizeof(void*)] = reinterpret_cast<void*>(&fake_get_back_buffer);
    surface_vtable[8 / sizeof(void*)] = reinterpret_cast<void*>(&fake_release);
    surface_vtable[0x30 / sizeof(void*)] = reinterpret_cast<void*>(&fake_get_desc);
    DAT_00cf1948 = &fake_swap_chain;

    // compute_dynamic_table_size() = DAT_008663d8*0x584 + 0x454 + DAT_00cf19d0,
    // read by initialize_default_render_state (pulled in transitively
    // through prepare_render_target_texture) at +0x474 for a capability
    // byte; point it at a real zero-filled scratch region rather than
    // leaving it at the default 0 (which would read unmapped memory).
    static alignas(16) std::uint8_t mode_table[0x584 + 0x475] = {};
    DAT_00cf19d0 = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(mode_table));

    // ---- no device yet: a no-op success -------------------------------------
    DAT_00cf1944 = nullptr;
    expect("no_device_is_noop_success", handle_device_lost() == 1);

    // ---- clean device: falls straight through to prepare_render_target ----
    DAT_00cf1944 = &fake_device;
    DAT_00cf1998 = 1; // disable the render-to-texture branch for simplicity
    test_level_result = 0; // S_OK
    DAT_00cf19dc = 0;
    test_level_calls = 0;
    create_vb_calls = 0;
    expect("clean_device_returns_success", handle_device_lost() == 1);
    expect("clean_device_tested_level", test_level_calls == 1);
    expect("clean_device_rebuilt_target", create_vb_calls >= 1);
    expect("clean_device_clears_flags", DAT_00cf19dc == 0 && DAT_00cf18c7 == 0);

    // ---- D3DERR_DEVICELOST: retry next iteration, no Reset ------------------
    test_level_result = -0x7789f798;
    reset_calls = 0;
    const std::uint32_t lost_result = handle_device_lost();
    expect("device_lost_no_reset_attempted", reset_calls == 0);
    expect("device_lost_returns_zero_low_byte", (lost_result & 0xffu) == 0);
    expect("device_lost_sets_in_progress_flag", DAT_00cf18c7 == 1);

    // ---- D3DERR_DEVICENOTRESET: release resources, Reset, rebuild ----------
    test_level_result = -0x7789f797;
    reset_result = 0; // S_OK
    reset_calls = 0;
    release_calls = 0;
    create_vb_calls = 0;
    DAT_00c75a7c = &fake_vertex_buffer; // pretend a vertex buffer exists
    expect("device_not_reset_succeeds", handle_device_lost() == 1);
    expect("device_not_reset_called_reset_once", reset_calls == 1);
    expect("device_not_reset_used_real_present_params", last_reset_params == DAT_00c75538);
    expect("device_not_reset_released_old_resources", release_calls >= 1);
    expect("device_not_reset_rebuilt_target", create_vb_calls >= 1);
    expect("device_not_reset_clears_flags", DAT_00cf19dc == 0 && DAT_00cf18c7 == 0);

    // ---- Reset() itself reports DEVICELOST: retry, no rebuild ---------------
    test_level_result = -0x7789f797;
    reset_result = -0x7789f798;
    reset_calls = 0;
    create_vb_calls = 0;
    const std::uint32_t reset_lost_result = handle_device_lost();
    expect("reset_device_lost_no_rebuild", create_vb_calls == 0);
    expect("reset_device_lost_returns_zero_low_byte", (reset_lost_result & 0xffu) == 0);

    // ---- set_render_targets_cached (0064c460) -------------------------------
    DAT_00bb0a40 = nullptr;
    DAT_00ba14f8 = nullptr;
    set_render_target_calls = 0;
    void* some_target = reinterpret_cast<void*>(0x1234);
    void* some_depth = reinterpret_cast<void*>(0x5678);
    set_render_targets_cached(some_target, some_depth);
    expect("cached_targets_applied_once", set_render_target_calls == 1);
    set_render_targets_cached(some_target, some_depth);
    expect("cached_targets_skipped_when_unchanged", set_render_target_calls == 1);

    if (failures != 0) {
        std::printf("device_lost_handler_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("device_lost_handler_smoke: all checks passed\n");
    return 0;
}
