// Local behavior checks for batch caps_table_057b: the SetRenderState /
// SetTextureStageState caches, the default render-state profile
// (verified 143/143 assignments against the exported decompiled C before
// integration), and the ESI-passing settings-apply function. Game-space
// symbols get test-local definitions.
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

void* DAT_00cf1944 = nullptr;
std::int32_t DAT_00c75188[256] = {};
std::int32_t DAT_009968d8[256 * 0x20] = {};
std::uint32_t DAT_00cf18c4 = 0;
std::uint32_t DAT_00cf18c8 = 0;
std::int32_t DAT_00cf18cc = 0;
std::int32_t DAT_00cf18e0 = 0;
std::uint32_t DAT_00881370[19] = {};
std::uint32_t DAT_008813bc = 0;
std::uint32_t DAT_008813c0 = 0;
std::uint32_t DAT_008813c4 = 0;
std::int32_t DAT_008663d8 = 0;
std::int32_t DAT_00cf19d0 = 0;
std::uint32_t DAT_00cf19a8 = 0;
std::int32_t DAT_0086527c = 0;
std::int32_t DAT_00865280 = 0;

// Pulled in transitively by hp6::render::prepare_render_target_texture,
// which calls reset_dynamic_resources / create_dynamic_vertex_buffer /
// reset_hook_placeholder (all in DeviceReset.obj alongside it).
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
struct ShaderSlot {
    std::uint32_t bytecode;
    std::uint32_t shader;
};
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

// Specific to prepare_render_target_texture itself.
void* DAT_00cf1948 = nullptr;
void* DAT_00cf1954 = nullptr;
void* DAT_00cf1958 = nullptr;
void* DAT_00cf195c = nullptr;
void* DAT_00bb0a40 = nullptr;
void* DAT_00ba14f8 = nullptr;
std::int32_t DAT_00cf1998 = 0;
std::uint8_t DAT_008663d4 = 0;
std::uint8_t DAT_00865287 = 0;
std::uint32_t DAT_008663b0 = 0;
std::uint32_t DAT_008663b4 = 0;
std::uint32_t DAT_00b170b8 = 0;

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

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

int render_state_calls = 0;
std::int32_t last_state = 0, last_state_value = 0;
std::int32_t HP6_STDCALL fake_set_render_state(void* device, std::int32_t state,
                                               std::int32_t value) {
    (void)device;
    ++render_state_calls;
    last_state = state;
    last_state_value = value;
    return 0;
}

int stage_state_calls = 0;
std::int32_t last_stage = 0, last_type = 0, last_stage_value = 0;
std::int32_t HP6_STDCALL fake_set_texture_stage_state(void* device, std::int32_t stage,
                                                      std::int32_t type,
                                                      std::int32_t value) {
    (void)device;
    ++stage_state_calls;
    last_stage = stage;
    last_type = type;
    last_stage_value = value;
    return 0;
}

// ---- prepare_render_target_texture (0x0064a810) fakes ------------------
int get_back_buffer_calls = 0;
void* back_buffer_surface_handle = nullptr;
std::int32_t HP6_STDCALL fake_get_back_buffer(void* swap_chain, std::uint32_t index,
                                              std::uint32_t type, void* out_surface) {
    (void)swap_chain;
    (void)index;
    (void)type;
    ++get_back_buffer_calls;
    *reinterpret_cast<void**>(out_surface) = back_buffer_surface_handle;
    return 0;
}

int get_desc_calls = 0;
std::uint32_t fake_surface_format = 0;
std::int32_t HP6_STDCALL fake_get_desc(void* surface, void* out_desc) {
    (void)surface;
    ++get_desc_calls;
    auto* words = static_cast<std::uint32_t*>(out_desc);
    words[0] = fake_surface_format;
    for (int i = 1; i < 8; ++i) words[i] = 0;
    return 0;
}

int release_surface_calls = 0;
void* last_released_surface = nullptr;
void HP6_STDCALL fake_release_surface(void* surface) {
    ++release_surface_calls;
    last_released_surface = surface;
}

int create_depth_stencil_calls = 0;
std::uint32_t last_ds_width = 0, last_ds_height = 0, last_ds_format = 0;
void* depth_stencil_handle = reinterpret_cast<void*>(0x7000);
std::int32_t HP6_STDCALL fake_create_depth_stencil(void* device, std::uint32_t width,
                                                   std::uint32_t height,
                                                   std::uint32_t format,
                                                   std::uint32_t multisample,
                                                   std::uint32_t msquality,
                                                   std::uint32_t discard,
                                                   void* out_surface,
                                                   std::uint32_t shared) {
    (void)device;
    (void)multisample;
    (void)msquality;
    (void)discard;
    (void)shared;
    ++create_depth_stencil_calls;
    last_ds_width = width;
    last_ds_height = height;
    last_ds_format = format;
    *reinterpret_cast<void**>(out_surface) = depth_stencil_handle;
    return 0;
}

int create_texture_calls = 0;
std::uint32_t last_tex_width = 0, last_tex_height = 0, last_tex_levels = 0,
              last_tex_usage = 0, last_tex_format = 0, last_tex_pool = 0;
std::int32_t create_texture_result = 0;
void* texture_handle = nullptr;
std::int32_t HP6_STDCALL fake_create_texture(void* device, std::uint32_t width,
                                             std::uint32_t height, std::uint32_t levels,
                                             std::uint32_t usage, std::uint32_t format,
                                             std::uint32_t pool, void* out_texture,
                                             std::uint32_t shared) {
    (void)device;
    (void)shared;
    ++create_texture_calls;
    last_tex_width = width;
    last_tex_height = height;
    last_tex_levels = levels;
    last_tex_usage = usage;
    last_tex_format = format;
    last_tex_pool = pool;
    *reinterpret_cast<void**>(out_texture) = texture_handle;
    return create_texture_result;
}

int get_surface_level_calls = 0;
void* texture_surface_handle = reinterpret_cast<void*>(0x8000);
std::int32_t HP6_STDCALL fake_get_surface_level(void* texture, std::uint32_t level,
                                                void* out_surface) {
    (void)texture;
    (void)level;
    ++get_surface_level_calls;
    *reinterpret_cast<void**>(out_surface) = texture_surface_handle;
    return 0;
}

int set_render_target_calls = 0;
void* last_render_target = nullptr;
std::int32_t HP6_STDCALL fake_set_render_target(void* device, std::uint32_t index,
                                                void* surface) {
    (void)device;
    (void)index;
    ++set_render_target_calls;
    last_render_target = surface;
    return 0;
}

int set_depth_stencil_calls = 0;
void* last_depth_stencil = nullptr;
std::int32_t HP6_STDCALL fake_set_depth_stencil(void* device, void* surface) {
    (void)device;
    ++set_depth_stencil_calls;
    last_depth_stencil = surface;
    return 0;
}

void* swap_chain_vtable[16] = {};
struct FakeSwapChain {
    void* vtable = swap_chain_vtable;
};
FakeSwapChain fake_swap_chain;

void* back_buffer_vtable[16] = {};
struct FakeBackBuffer {
    void* vtable = back_buffer_vtable;
};
FakeBackBuffer fake_back_buffer;

void* texture_vtable[24] = {};
struct FakeTexture {
    void* vtable = texture_vtable;
};
FakeTexture fake_texture;

void* device_vtable[112] = {};
struct FakeDevice {
    void* vtable = device_vtable;
};
FakeDevice fake_device;

} // namespace

int main() {
    using namespace hp6::render;

    device_vtable[0xe4 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_render_state);
    device_vtable[0x10c / sizeof(void*)] =
        reinterpret_cast<void*>(&fake_set_texture_stage_state);
    DAT_00cf1944 = &fake_device;

    // ---- set_render_state_cached (0064c300) --------------------------------
    std::memset(DAT_00c75188, 0, sizeof DAT_00c75188);
    set_render_state_cached(5, 10);
    expect("render_state_first_call", render_state_calls == 1 && last_state == 5 &&
                                      last_state_value == 10);
    render_state_calls = 0;
    set_render_state_cached(5, 10); // unchanged: no call
    expect("render_state_cached_skip", render_state_calls == 0);
    set_render_state_cached(5, 11);
    expect("render_state_changed_call", render_state_calls == 1 && last_state_value == 11);

    // ---- set_texture_stage_state_cached (0064c3b0) -------------------------
    std::memset(DAT_009968d8, 0, sizeof DAT_009968d8);
    set_texture_stage_state_cached(2, 3, 7);
    expect("stage_state_first_call", stage_state_calls == 1 && last_stage == 2 &&
                                     last_type == 3 && last_stage_value == 7);
    stage_state_calls = 0;
    set_texture_stage_state_cached(2, 3, 7);
    expect("stage_state_cached_skip", stage_state_calls == 0);
    set_texture_stage_state_cached(2, 3, 9);
    expect("stage_state_changed_call", stage_state_calls == 1 && last_stage_value == 9);
    set_texture_stage_state_cached(3, 3, 9); // different stage: distinct cache slot
    expect("stage_state_distinct_slot", stage_state_calls == 2);

    // ---- apply_render_state_profile (010021f0) -----------------------------
    std::memset(DAT_00c75188, 0, sizeof DAT_00c75188);
    std::memset(DAT_009968d8, 0, sizeof DAT_009968d8);
    render_state_calls = 0;
    stage_state_calls = 0;
    std::uint32_t settings[19];
    for (std::uint32_t i = 0; i < 19; ++i) settings[i] = 100 + i;
    apply_render_state_profile(settings);
    expect("profile_render_state_count", render_state_calls == 12);
    expect("profile_stage_state_count", stage_state_calls == 6);
    expect("profile_last_render_state", last_state == 0xab && last_state_value == 112);
    expect("profile_last_stage_state",
           last_stage == 0 && last_type == 6 && last_stage_value == 118);
    expect("profile_persisted_first", DAT_00881370[1] == settings[1]);
    expect("profile_persisted_last", DAT_00881370[18] == settings[18]);
    expect("profile_sentinel", DAT_0086527c == static_cast<std::int32_t>(settings[0]));

    // ---- initialize_default_render_state (00640fb0) ------------------------
    std::memset(DAT_00c75188, 0, sizeof DAT_00c75188);
    std::memset(DAT_009968d8, 0, sizeof DAT_009968d8);
    DAT_008663d8 = 0;
    DAT_00cf19a8 = 0;
    // The capability byte at mode+0x474: bit 0x80 clear triggers the
    // adjusted block.
    alignas(16) std::uint8_t mode_table[0x584 + 0x475] = {};
    // DAT_00cf19d0 holds the mode table's base address as a plain integer
    // (read directly, not as &DAT_00cf19d0); point it at a zero-filled
    // scratch region so capability bit 0x80 reads clear.
    DAT_00cf19d0 = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(mode_table));
    initialize_default_render_state();
    expect("init_bit80_clear_adjust", DAT_00f82d24 == 1 && DAT_00f82d28 == 2);
    expect("init_unconditional_tail", DAT_00f83154 == 1 && DAT_00f83108 == 1);
    expect("init_sentinel_sticky", DAT_008813bc == 0xcdcdcdcdu);
    expect("init_final_overwrite", DAT_0086527c == -8 && DAT_00865280 == -7);
    expect("init_profile_applied", render_state_calls > 0 || stage_state_calls > 0);

    // ---- prepare_render_target_texture (0x0064a810) ------------------------
    // Every vtable offset below was cross-checked against the real
    // IDirect3DSwapChain9/IDirect3DSurface9/IDirect3DTexture9/
    // IDirect3DDevice9 layouts; see src/render/DeviceReset.cpp.
    swap_chain_vtable[0x14 / sizeof(void*)] = reinterpret_cast<void*>(&fake_get_back_buffer);
    back_buffer_vtable[8 / sizeof(void*)] = reinterpret_cast<void*>(&fake_release_surface);
    back_buffer_vtable[0x30 / sizeof(void*)] = reinterpret_cast<void*>(&fake_get_desc);
    device_vtable[0x74 / sizeof(void*)] = reinterpret_cast<void*>(&fake_create_depth_stencil);
    device_vtable[0x5c / sizeof(void*)] = reinterpret_cast<void*>(&fake_create_texture);
    device_vtable[0x94 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_render_target);
    device_vtable[0x9c / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_depth_stencil);
    texture_vtable[0x48 / sizeof(void*)] = reinterpret_cast<void*>(&fake_get_surface_level);

    DAT_00cf1948 = &fake_swap_chain;
    back_buffer_surface_handle = &fake_back_buffer;
    texture_handle = &fake_texture;
    DAT_00c75a7c = reinterpret_cast<void*>(0x9999); // already created: skip that path
    DAT_00cf1a30 = 0;                               // no shaders to recreate
    DAT_008663b0 = 1024;
    DAT_008663b4 = 768;
    DAT_00b170b8 = 0x15; // an arbitrary D3DFORMAT for the depth-stencil request
    fake_surface_format = 0x16; // the back buffer's reported format

    // DAT_00cf1998 != 0: render-to-texture disabled outright.
    DAT_00cf1998 = 1;
    create_texture_calls = 0;
    DAT_00cf1958 = nullptr;
    prepare_render_target_texture();
    expect("rtt_back_buffer_fetched", get_back_buffer_calls == 1);
    // The disabled path never reaches CreateTexture, so the back buffer
    // surface GetBackBuffer just fetched is never released (matching the
    // exported x86: Release only runs after a successful CreateTexture).
    expect("rtt_disabled_no_release", release_surface_calls == 0);
    expect("rtt_depth_stencil_created", create_depth_stencil_calls == 1 &&
                                        last_ds_width == 1024 && last_ds_height == 768 &&
                                        last_ds_format == 0x15);
    expect("rtt_disabled_sentinel",
           DAT_00cf1958 == reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xbacb0ffeu)));
    expect("rtt_disabled_no_texture", create_texture_calls == 0);
    expect("rtt_render_target_bound", set_render_target_calls == 1);
    expect("rtt_depth_stencil_bound", set_depth_stencil_calls == 1 &&
                                      last_depth_stencil == depth_stencil_handle);

    // DAT_00cf1998 == 0 and both feature flags set: texture path runs.
    DAT_00cf1998 = 0;
    DAT_008663d4 = 1;
    DAT_00865287 = 1;
    alignas(16) std::uint8_t capability_table[0x584 * 2] = {};
    // DAT_008663d8 selects the mode row; compute_dynamic_table_size()
    // returns DAT_008663d8*0x584 + 0x454 + DAT_00cf19d0, and this test
    // reads capability_record+0x3c from that address, i.e. byte offset
    // 0x454+0x3c = 0x490 from DAT_00cf19d0's own value.
    DAT_008663d8 = 0;
    DAT_00cf19d0 = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(capability_table));
    *reinterpret_cast<std::uint32_t*>(capability_table + 0x490) = 0x100; // bit 0x100 set: do not skip
    get_back_buffer_calls = 0;
    release_surface_calls = 0;
    create_texture_calls = 0;
    get_surface_level_calls = 0;
    create_texture_result = 0;
    DAT_00cf1954 = nullptr;
    DAT_00cf1958 = nullptr;
    prepare_render_target_texture();
    expect("rtt_texture_created", create_texture_calls == 1 && last_tex_width == 1024 &&
                                  last_tex_height == 768 && last_tex_levels == 1 &&
                                  last_tex_usage == 1 && last_tex_format == 0x16 &&
                                  last_tex_pool == 0);
    expect("rtt_old_back_buffer_released", release_surface_calls == 1);
    expect("rtt_surface_level_fetched", get_surface_level_calls == 1);
    expect("rtt_texture_cached", DAT_00cf1958 == &fake_texture);
    expect("rtt_texture_surface_cached", DAT_00cf1954 == texture_surface_handle);
    expect("rtt_texture_bound_as_target", last_render_target == texture_surface_handle);

    // Capability bits clear the "skip" condition's complement: both 0x100
    // clear and 0x2 set means skip texture creation even though enabled.
    *reinterpret_cast<std::uint32_t*>(capability_table + 0x490) = 0x2;
    create_texture_calls = 0;
    prepare_render_target_texture();
    expect("rtt_capability_skip", create_texture_calls == 0);

    if (failures != 0) {
        std::printf("render_state_cache_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("render_state_cache_smoke: all checks passed\n");
    return 0;
}
