// Local behavior check for hp6::render::submit_screen_quad (0x00648c00)
// and render_fade_overlay_quad (0x0064abd0) — the real Direct3D 9
// screen-quad composite pipeline, reached in the exported x86 only
// from the (confirmed hardcoded-false-in-FINAL-build) debug overlay
// chain, but reconstructed faithfully from real disassembly: locking
// the shared dynamic vertex buffer, filling real vertex data, binding
// it, and issuing a real DrawPrimitive call on a fake IDirect3DDevice9,
// followed by EndScene/Present. See architecture/startup_path.md
// ("iMsgDoRender / GUI rendering trail") for the full investigation
// this reconstruction grew out of.
#include "render/DeviceLostHandler.h"
#include "render/RenderStateCache.h"
#include "render/ScreenOverlayRender.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#define HP6_STDCALL __stdcall
#else
#define HP6_STDCALL
#endif

// ---- RenderStateCache.cpp / DeviceReset.cpp / RenderTargetSetup.cpp's own
// transitive extern dependencies (same block tests/device_lost_handler_
// smoke.cpp already establishes, for the same reason: linking
// initialize_default_render_state's huge default-table pulls all of this
// in whether or not this test's own two scenarios reach it) -----------------
void* DAT_00cf1944 = nullptr;
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
void* DAT_00bb0a40 = nullptr;
void* DAT_00ba14f8 = nullptr;
std::int32_t DAT_00cf1998 = 0;
std::uint8_t DAT_008663d4 = 0;
std::uint8_t DAT_00865287 = 0;
std::uint32_t DAT_008663b0 = 640;
std::uint32_t DAT_008663b4 = 480;

// ---- RenderStateCache.cpp's new cached-setter extern dependencies ---------
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

// ---- ScreenOverlayRender.cpp's own extern dependencies ---------------------
void* DAT_00cf1930 = reinterpret_cast<void*>(0x1234); // opaque vertex-declaration handle
void* DAT_00cf192c = reinterpret_cast<void*>(0x5678); // opaque vertex-shader handle
std::uint32_t DAT_00866358 = 0;
std::uint32_t DAT_00cf1908 = 0;
std::uint32_t DAT_00866360 = 0;
std::uint32_t DAT_00cf19e4 = 0;
std::uint32_t DAT_00cf19e0 = 0;
std::int32_t DAT_00cf1994 = 0;
float DAT_0080a8b0 = 1.0f;
float DAT_0080a8d0 = 0.0f;
float DAT_00831c2c = 0.0f;
float DAT_007db234 = 1.0f;

// ---- DeviceLostHandler.h pulls in handle_device_lost/shutdown_render_device
// too (same translation unit), even though this test only exercises
// render_fade_overlay_quad ---------------------------------------------------
std::uint8_t DAT_00cf19dc = 0;
std::uint8_t DAT_00cf18c7 = 0;
std::uint32_t DAT_00c75538[14] = {};
void* DAT_00cf19d8 = nullptr;
void* DAT_00cf194c = nullptr;
std::uint32_t DAT_00cf19c8 = 0;
std::uint32_t DAT_00b170b8 = 0;

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

// ---- fake Direct3D 9 device chain ------------------------------------------
void* device_vtable[0x1a0 / sizeof(void*) + 2] = {};
struct FakeDevice { void* vtable = device_vtable; };
FakeDevice fake_device;

void* swap_chain_vtable[8] = {};
struct FakeSwapChain { void* vtable = swap_chain_vtable; };
FakeSwapChain fake_swap_chain;

// Sized for GetDesc (0x30), not just Release (8): handle_device_lost's
// "clean device" path calls prepare_render_target_texture, which reads
// the back buffer's own surface description (see feedback memory on
// fake-vtable sizing: undersizing this corrupts memory silently rather
// than crashing predictably).
void* surface_vtable[16] = {};
struct FakeSurface { void* vtable = surface_vtable; };
FakeSurface fake_back_buffer;

int get_desc_calls = 0;
std::int32_t HP6_STDCALL fake_get_desc(void*, void* out) {
    ++get_desc_calls;
    std::memset(out, 0, 0x20);
    return 0;
}

void* texture_vtable[4] = {};
struct FakeTexture { void* vtable = texture_vtable; };
FakeTexture fake_render_target_texture;

std::uint8_t vb_backing[0x2000];
int lock_calls = 0, unlock_calls = 0;
std::int32_t HP6_STDCALL fake_vb_lock(void*, std::uint32_t, std::uint32_t, void** out, std::uint32_t) {
    ++lock_calls;
    *out = vb_backing;
    return 0;
}
std::int32_t HP6_STDCALL fake_vb_unlock(void*) { ++unlock_calls; return 0; }
void* vertex_buffer_vtable[0x31 / sizeof(void*) + 1] = {};
struct FakeVertexBuffer { void* vtable = vertex_buffer_vtable; };
FakeVertexBuffer fake_vertex_buffer;

int set_stream_source_calls = 0;
void* last_stream_source_buffer = nullptr;
std::uint32_t last_stream_source_stride = 0;
std::int32_t HP6_STDCALL fake_set_stream_source(void*, std::uint32_t, void* buffer, std::uint32_t,
                                                std::uint32_t stride) {
    ++set_stream_source_calls;
    last_stream_source_buffer = buffer;
    last_stream_source_stride = stride;
    return 0;
}

int draw_primitive_calls = 0;
std::uint32_t last_primitive_type = 0, last_start_vertex = 0, last_primitive_count = 0;
std::int32_t HP6_STDCALL fake_draw_primitive(void*, std::uint32_t type, std::uint32_t start,
                                             std::uint32_t count) {
    ++draw_primitive_calls;
    last_primitive_type = type;
    last_start_vertex = start;
    last_primitive_count = count;
    return 0;
}

int end_scene_calls = 0;
std::int32_t HP6_STDCALL fake_end_scene(void*) { ++end_scene_calls; return 0; }
int present_calls = 0;
std::int32_t HP6_STDCALL fake_present(void*, void*, void*, void*, void*) { ++present_calls; return 0; }

int get_back_buffer_calls = 0;
std::int32_t HP6_STDCALL fake_get_back_buffer(void*, std::uint32_t, std::uint32_t, void* out) {
    ++get_back_buffer_calls;
    *reinterpret_cast<void**>(out) = &fake_back_buffer;
    return 0;
}
int release_calls = 0;
void HP6_STDCALL fake_release(void*) { ++release_calls; }

int set_render_target_calls = 0;
std::int32_t HP6_STDCALL fake_set_render_target(void*, std::uint32_t, void*) {
    ++set_render_target_calls;
    return 0;
}
std::int32_t HP6_STDCALL fake_set_depth_stencil(void*, void*) { return 0; }
std::int32_t HP6_STDCALL fake_set_render_state(void*, std::uint32_t, std::uint32_t) { return 0; }

int set_texture_calls = 0;
void* last_texture = nullptr;
std::int32_t HP6_STDCALL fake_set_texture(void*, std::uint32_t, void* texture) {
    ++set_texture_calls;
    last_texture = texture;
    return 0;
}
int set_vertex_declaration_calls = 0;
std::int32_t HP6_STDCALL fake_set_vertex_declaration(void*, void*) {
    ++set_vertex_declaration_calls;
    return 0;
}
int set_vertex_shader_calls = 0;
std::int32_t HP6_STDCALL fake_set_vertex_shader(void*, void*) {
    ++set_vertex_shader_calls;
    return 0;
}

int begin_scene_calls = 0;
std::int32_t HP6_STDCALL fake_begin_scene(void*) { ++begin_scene_calls; return 0; }

int test_cooperative_level_calls = 0;
std::int32_t test_cooperative_level_result = 0;
std::int32_t HP6_STDCALL fake_test_cooperative_level(void*) {
    ++test_cooperative_level_calls;
    return test_cooperative_level_result;
}

std::uint32_t HP6_STDCALL fake_get_available_texture_mem(void*) { return 0; }
std::int32_t HP6_STDCALL fake_set_texture_stage_state(void*, std::uint32_t, std::uint32_t,
                                                       std::uint32_t) {
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

} // namespace

int main() {
    using namespace hp6::render;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    vertex_buffer_vtable[0x2c / sizeof(void*)] = reinterpret_cast<void*>(&fake_vb_lock);
    vertex_buffer_vtable[0x30 / sizeof(void*)] = reinterpret_cast<void*>(&fake_vb_unlock);
    DAT_00c75a7c = &fake_vertex_buffer;

    device_vtable[400 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_stream_source);
    device_vtable[0x144 / sizeof(void*)] = reinterpret_cast<void*>(&fake_draw_primitive);
    device_vtable[0xa8 / sizeof(void*)] = reinterpret_cast<void*>(&fake_end_scene);
    device_vtable[0x44 / sizeof(void*)] = reinterpret_cast<void*>(&fake_present);
    device_vtable[0x94 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_render_target);
    device_vtable[0x9c / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_depth_stencil);
    device_vtable[0xe4 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_render_state);
    device_vtable[0x104 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_texture);
    device_vtable[0x15c / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_vertex_declaration);
    device_vtable[0x170 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_vertex_shader);
    device_vtable[0xa4 / sizeof(void*)] = reinterpret_cast<void*>(&fake_begin_scene);
    device_vtable[0xc / sizeof(void*)] = reinterpret_cast<void*>(&fake_test_cooperative_level);
    device_vtable[0x10 / sizeof(void*)] = reinterpret_cast<void*>(&fake_get_available_texture_mem);
    device_vtable[0x10c / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_texture_stage_state);
    device_vtable[0x74 / sizeof(void*)] = reinterpret_cast<void*>(&fake_create_depth_stencil);

    // initialize_default_render_state (pulled in transitively through
    // prepare_render_target_texture) reads a capability byte at
    // DAT_00cf19d0 + DAT_008663d8*0x584 + 0x474; point it at a real
    // zero-filled scratch region rather than leaving it at the default 0
    // (which would read unmapped memory), same fix as
    // tests/device_lost_handler_smoke.cpp already applies.
    static alignas(16) std::uint8_t mode_table[0x584 + 0x475] = {};
    DAT_00cf19d0 = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(mode_table));
    DAT_00cf1944 = &fake_device;

    swap_chain_vtable[0x14 / sizeof(void*)] = reinterpret_cast<void*>(&fake_get_back_buffer);
    DAT_00cf1948 = &fake_swap_chain;

    surface_vtable[8 / sizeof(void*)] = reinterpret_cast<void*>(&fake_release);
    surface_vtable[0x30 / sizeof(void*)] = reinterpret_cast<void*>(&fake_get_desc);
    texture_vtable[8 / sizeof(void*)] = reinterpret_cast<void*>(&fake_release);

    // ---- submit_screen_quad (00648c00) -------------------------------------
    const float verts[4 * 4] = {
        // x, y, z, rhw per vertex (flags=1, position only, 4 verts)
        0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 0.0f, 1.0f,
    };
    submit_screen_quad(verts, 0x1 /*position only*/, 4);
    expect("quad_locked_and_unlocked", lock_calls == 1 && unlock_calls == 1);
    expect("quad_bound_stream_source",
           set_stream_source_calls == 1 && last_stream_source_buffer == &fake_vertex_buffer &&
               last_stream_source_stride == 0x1c);
    expect("quad_set_declaration_and_shader",
           set_vertex_declaration_calls == 1 && set_vertex_shader_calls == 1);
    expect("quad_drew_two_triangles",
           draw_primitive_calls == 1 && last_primitive_type == 5 && last_start_vertex == 0 &&
               last_primitive_count == 2);
    // Position-only vertices (flags bit0 only): colour defaults to opaque
    // white, UV defaults to (0,0), matching the exported x86's own
    // defaults.
    auto* written = reinterpret_cast<const std::uint32_t*>(vb_backing);
    expect("quad_default_colour_is_opaque_white", written[4] == 0xffffffffu);
    expect("quad_default_uv_is_zero", written[5] == 0 && written[6] == 0);

    // ---- render_fade_overlay_quad (0064abd0): disabled, zero countdown ----
    lock_calls = 0;
    DAT_00cf19e4 = 0;
    render_fade_overlay_quad(0x80ffffffu);
    expect("fade_noop_when_countdown_zero", lock_calls == 0 && end_scene_calls == 0);

    // ---- render_fade_overlay_quad: armed, render target texture active ----
    DAT_00cf19e4 = 1;
    DAT_00cf19e0 = 128;
    DAT_00cf1958 = &fake_render_target_texture;
    lock_calls = 0;
    unlock_calls = 0;
    draw_primitive_calls = 0;
    get_back_buffer_calls = 0;
    release_calls = 0;
    set_texture_calls = 0;
    end_scene_calls = 0;
    present_calls = 0;

    render_fade_overlay_quad(0x80ffffffu);
    expect("fade_countdown_decremented", DAT_00cf19e4 == 0);
    expect("fade_fetched_back_buffer_and_released_it", get_back_buffer_calls == 1 && release_calls == 1);
    expect("fade_bound_render_target_texture_as_texture",
           set_texture_calls >= 1 && last_texture == &fake_render_target_texture);
    expect("fade_drew_the_quad", draw_primitive_calls == 1 && last_primitive_count == 2);
    expect("fade_ended_scene_and_presented", end_scene_calls == 1 && present_calls == 1);
    expect("fade_clears_its_own_countdown_value", DAT_00cf19e0 == 0);

    // The 4 vertices' colour field (index 4 of each 7-word vertex) must
    // carry the real `color` argument through unchanged.
    auto* fade_written = reinterpret_cast<const std::uint32_t*>(vb_backing);
    bool colour_ok = true;
    for (int v = 0; v < 4; ++v) {
        if (fade_written[v * 7 + 4] != 0x80ffffffu) colour_ok = false;
    }
    expect("fade_quad_colour_matches_argument", colour_ok);

    // ---- render_fade_overlay_quad: armed, but render target texture disabled --
    DAT_00cf19e4 = 1;
    DAT_00cf1958 = nullptr;
    draw_primitive_calls = 0;
    end_scene_calls = 0;
    present_calls = 0;
    render_fade_overlay_quad(0xffffffffu);
    expect("fade_skips_quad_when_texture_disabled", draw_primitive_calls == 0);
    expect("fade_still_ends_scene_and_presents", end_scene_calls == 1 && present_calls == 1);

    // ---- try_begin_debug_overlay_scene (0064a9e0) --------------------------
    // Device lost: handle_device_lost reports not-ready, no real BeginScene,
    // the fade countdown stays unarmed.
    test_cooperative_level_result = -0x7789f798; // D3DERR_DEVICELOST
    begin_scene_calls = 0;
    DAT_00cf19e4 = 0;
    bool began = try_begin_debug_overlay_scene();
    expect("overlay_scene_not_begun_when_device_lost", !began && begin_scene_calls == 0);
    expect("overlay_scene_countdown_unchanged_when_not_begun", DAT_00cf19e4 == 0);

    // Device ready: the real BeginScene fires and arms render_fade_overlay_
    // quad's own countdown by one.
    test_cooperative_level_result = 0; // S_OK
    begin_scene_calls = 0;
    began = try_begin_debug_overlay_scene();
    expect("overlay_scene_begun_when_device_ready", began && begin_scene_calls == 1);
    expect("overlay_scene_arms_fade_countdown", DAT_00cf19e4 == 1);

    if (failures != 0) {
        std::printf("screen_overlay_render_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("screen_overlay_render_smoke: all checks passed\n");
    return 0;
}
