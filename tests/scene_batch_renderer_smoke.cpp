// Local behavior check for hp6::render::execute_scene_render_batch
// (0x00646200, confirmed "Scene::End" via its own embedded diagnostic
// string). The real per-item draw executor at the end of this
// project's traced UI/scene render chain -- see
// architecture/startup_path.md ("iMsgDoRender / GUI rendering trail",
// suite 14) for the full chain this grew out of
// (PluginPresentationBitmap_BuildRenderQuad -> ... -> SubmitRenderCommand
// -> Scene::RenderAllRegions -> this function).
#include "render/RenderStateCache.h"
#include "render/SceneBatchRenderer.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#define HP6_STDCALL __stdcall
#else
#define HP6_STDCALL
#endif

// ---- RenderStateCache.cpp's own extern dependencies (the small set
// SceneBatchRenderer.cpp's own real helpers pull in; the big
// render-state-default-table block is not needed since this test does
// not link DeviceCreate.obj/DeviceReset.obj) --------------------------
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
std::int32_t DAT_00b108a0[256] = {};
std::uint32_t DAT_00bb09b4[16] = {};
void* DAT_00cf19c4 = nullptr;
std::uint32_t DAT_008663cc = 0;
std::uint32_t DAT_008663d0 = 0;
std::uint32_t DAT_0096b484 = 0;
std::uint32_t DAT_00bac98c = 0;
std::uint8_t DAT_00a70890[0x14 * 4] = {};
std::uint8_t DAT_00baa170[0x14 * 4] = {};
std::uint8_t DAT_0098c6a8[0x14 * 4] = {};
std::uint8_t DAT_009985a0[0x14 * 4] = {};
std::int32_t DAT_00998580 = 0;
std::int32_t DAT_00c754cc = 0;
std::uint8_t DAT_00f38b28[0xc4] = {};
std::int32_t DAT_00cf1a3c = 0;
std::int32_t DAT_00cf1a48 = 0;
std::uint32_t DAT_00b170c4 = 0;

// ---- SceneBatchRenderer.cpp's own extern dependencies ----------------
std::uint8_t DAT_008663d5 = 0;
std::uint32_t DAT_00b10ce4 = 0;
std::uint32_t DAT_009968c4 = 0;
std::int32_t DAT_00cf18ec = 0;
std::uint32_t DAT_00866358 = 0;
std::uint32_t DAT_00cf1908 = 0;
std::uint32_t DAT_0086635c = 0;
std::uint32_t DAT_00866360 = 0;
std::uint32_t DAT_00ba0cf0 = 0;
std::uint8_t DAT_00cf1895 = 0;
std::uint32_t DAT_009968ac = 0;
std::uint32_t DAT_009968b0 = 0, DAT_009968b4 = 0, DAT_009968b8 = 0;
std::uint32_t DAT_009968c8 = 0, DAT_009968cc = 0;
std::uint8_t DAT_00865285 = 1;

// ---- push_item_transform_and_material's own extern dependencies -----
std::int32_t DAT_00c76680 = 0;
std::uint32_t DAT_00ba14fc = 0;
std::int32_t DAT_00c76684 = 0;
std::uint8_t DAT_00f82530[0x40] = {}; // one region's worth is enough: this test only uses region 0
std::uint8_t DAT_00f82310[0x40] = {};
std::int32_t DAT_00cf1914 = 0;
std::uint32_t DAT_0099858c = 0;
std::int32_t DAT_00c76698 = 0;
std::int32_t DAT_00c7668c = 0;
std::uint32_t DAT_00c766a8 = 0;
std::int32_t DAT_00c76690 = 0;
std::uint32_t DAT_00c766ac = 0;
float DAT_00848478 = 0.5f;
float DAT_007db234 = 1.0f;
float DAT_00831c2c = 0.0f;
float DAT_008483dc = 1.0f / 255.0f;
float DAT_008483f0 = 1.0f / 255.0f;

// ---- resolve_material_shader_effect's own extern dependencies --------
// Sized to cover group in {0, 1} (the only values resolve_material_bindings
// ever passes) plus slack for any variant/index combination the formula
// `variant*0xe + group*0x1816c + index` can produce.
void* DAT_008a6750[0x1816cu * 2 + 0x100] = {};
void* DAT_008a6478[0x1816cu * 2 + 0x100] = {};

// ---- resolve_material_bindings's own extern dependencies -------------
float DAT_00848418 = 0.25f;
void* DAT_00cf1934 = reinterpret_cast<void*>(0xaaaa0002u);
void* DAT_00cf1924 = reinterpret_cast<void*>(0xaaaa0003u);
void* DAT_0098c69c = reinterpret_cast<void*>(0xaaaa000au);
void* DAT_00cf1928 = reinterpret_cast<void*>(0xaaaa000bu);
void* DAT_00cf1930 = reinterpret_cast<void*>(0xaaaa0108u);
void* DAT_00cf192c = reinterpret_cast<void*>(0xaaaa0109u);

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

void* device_vtable[0x1b0 / sizeof(void*) + 1] = {};
struct FakeDevice { void* vtable = device_vtable; };
FakeDevice fake_device;

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

int draw_indexed_calls = 0;
std::uint32_t last_base_vertex = 0, last_min_vertex = 0, last_num_vertices = 0, last_start_index = 0,
              last_indexed_primitive_count = 0;
std::int32_t HP6_STDCALL fake_draw_indexed_primitive(void*, std::uint32_t type,
                                                      std::int32_t base_vertex,
                                                      std::uint32_t min_vertex,
                                                      std::uint32_t num_vertices,
                                                      std::uint32_t start_index,
                                                      std::uint32_t primitive_count) {
    ++draw_indexed_calls;
    last_primitive_type = type;
    last_base_vertex = static_cast<std::uint32_t>(base_vertex);
    last_min_vertex = min_vertex;
    last_num_vertices = num_vertices;
    last_start_index = start_index;
    last_indexed_primitive_count = primitive_count;
    return 0;
}

int set_stream_source_calls = 0;
std::int32_t HP6_STDCALL fake_set_stream_source(void*, std::uint32_t, void*, std::uint32_t,
                                                std::uint32_t) {
    ++set_stream_source_calls;
    return 0;
}
int set_indices_calls = 0;
void* last_indices = nullptr;
std::int32_t HP6_STDCALL fake_set_indices(void*, void* indices) {
    ++set_indices_calls;
    last_indices = indices;
    return 0;
}
int set_pixel_shader_calls = 0;
std::int32_t HP6_STDCALL fake_set_pixel_shader(void*, void*) {
    ++set_pixel_shader_calls;
    return 0;
}

// push_item_transform_and_material/resolve_material_bindings's own
// SetVertexShaderConstantF calls (device vtable+0x178) -- captures the
// most recent call's register/data/count so tests can check the real
// transform math and constant groups it computes.
int set_vs_const_calls = 0;
std::uint32_t last_vs_const_register = 0, last_vs_const_count = 0;
// Captured per-register (not just "last call overall") since a single
// push_item_transform_and_material call pushes several different
// registers (0, 4, 5, 6, 0x11, 0x12) in sequence.
int vs_const_calls_for_register[32] = {};
float vs_const_data_for_register[32][36] = {};
std::int32_t HP6_STDCALL fake_set_vertex_shader_constant_f(void*, std::uint32_t start_register,
                                                           const void* data, std::uint32_t count) {
    ++set_vs_const_calls;
    last_vs_const_register = start_register;
    last_vs_const_count = count;
    const auto* floats = static_cast<const float*>(data);
    const std::size_t words = static_cast<std::size_t>(count) * 4;
    if (start_register < 32) {
        ++vs_const_calls_for_register[start_register];
        for (std::size_t i = 0; i < words && i < 36; ++i) {
            vs_const_data_for_register[start_register][i] = floats[i];
        }
    }
    return 0;
}

int set_render_state_calls = 0;
std::int32_t last_render_state = 0, last_render_state_value = 0;
std::int32_t HP6_STDCALL fake_set_render_state(void*, std::int32_t state, std::int32_t value) {
    ++set_render_state_calls;
    last_render_state = state;
    last_render_state_value = value;
    return 0;
}

int set_vertex_declaration_calls = 0;
void* last_vertex_declaration = nullptr;
std::int32_t HP6_STDCALL fake_set_vertex_declaration(void*, void* declaration) {
    ++set_vertex_declaration_calls;
    last_vertex_declaration = declaration;
    return 0;
}
int set_vertex_shader_calls = 0;
void* last_vertex_shader = nullptr;
std::int32_t HP6_STDCALL fake_set_vertex_shader(void*, void* shader) {
    ++set_vertex_shader_calls;
    last_vertex_shader = shader;
    return 0;
}

// A real vertex/index buffer pointer stored in the handle-resolution
// tables, so bind_vertex_buffer_by_handle/bind_index_buffer_by_handle
// resolve to something real rather than garbage.
void* fake_vertex_buffer_object = reinterpret_cast<void*>(0x1234);
void* fake_index_buffer_object = reinterpret_cast<void*>(0x5678);

} // namespace

int main() {
    using namespace hp6::render;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    device_vtable[0x144 / sizeof(void*)] = reinterpret_cast<void*>(&fake_draw_primitive);
    device_vtable[0x148 / sizeof(void*)] = reinterpret_cast<void*>(&fake_draw_indexed_primitive);
    device_vtable[400 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_stream_source);
    device_vtable[0x1a0 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_indices);
    device_vtable[0x1ac / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_pixel_shader);
    device_vtable[0x178 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_vertex_shader_constant_f);
    device_vtable[0x15c / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_vertex_declaration);
    device_vtable[0x170 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_vertex_shader);
    device_vtable[0xe4 / sizeof(void*)] = reinterpret_cast<void*>(&fake_set_render_state);
    DAT_00cf1944 = &fake_device;
    for (auto& entry : DAT_00c75188) { entry = -1; } // force cache-miss for every state

    // Handle 0 resolves through the ">= 0" table (DAT_00a70890/
    // DAT_0098c6a8), field+0x10, matching bind_vertex_buffer_by_handle/
    // bind_index_buffer_by_handle's own real offset math.
    *reinterpret_cast<void**>(DAT_00a70890 + 0x10) = fake_vertex_buffer_object;
    *reinterpret_cast<void**>(DAT_0098c6a8 + 0x10) = fake_index_buffer_object;

    // ---- low-tier path (DAT_008663d5 != 0): DrawPrimitive, no index --
    DAT_008663d5 = 1;
    // Force the "cache miss" branch on every binding cache below (their
    // real defaults happen to be 0, the same as the handles this test
    // uses, which would otherwise make every cached setter think the
    // real binding was already in place and skip the call).
    DAT_00866358 = 0xffffffffu;
    DAT_00cf1908 = 0xffffffffu;
    DAT_0086635c = 0xffffffffu;
    DAT_008663d0 = 0xffffffffu;
    std::uint8_t item[0x28] = {};
    *reinterpret_cast<std::uint32_t*>(item + 0x10) = 0;          // vertex buffer handle 0
    *reinterpret_cast<std::uint32_t*>(item + 0x14) = 0xbad1bad1u; // no index buffer
    *reinterpret_cast<std::uint16_t*>(item + 0x1e) = 2;          // primitive count
    *reinterpret_cast<std::uint16_t*>(item + 0x20) = 7;          // start vertex
    item[0x25] = 4;                                              // primitive type (D3DPT_TRIANGLELIST)
    *reinterpret_cast<std::uint32_t*>(item + 0x18) = 2;          // material type 2 (recognized)

    // Region 0's view/projection matrix: identity, so the real multiply
    // in push_item_transform_and_material leaves the item's own
    // transform unchanged, letting this test check the real transpose
    // directly against a known, distinguishable input matrix below.
    const float identity_region_matrix[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    std::memcpy(DAT_00f82530, identity_region_matrix, sizeof(identity_region_matrix));
    DAT_00cf1914 = 0;

    // A translation-only transform (row-major, row-vector convention:
    // translation lives in row 3) -- its transpose moves the translation
    // into column 3, which is what register 0 should receive.
    const float item_transform[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 10, 20, 30, 1};

    std::int32_t context[16] = {};
    context[0] = static_cast<std::int32_t>(reinterpret_cast<std::intptr_t>(item_transform));
    execute_scene_render_batch(context, item, 0, 0, nullptr);
    expect("low_tier_draws_primitive", draw_primitive_calls == 1 && draw_indexed_calls == 0);
    expect("low_tier_primitive_args",
           last_primitive_type == 4 && last_start_vertex == 7 && last_primitive_count == 2);
    expect("low_tier_bound_vertex_stream", set_stream_source_calls == 1);
    expect("low_tier_set_pixel_shader_null", set_pixel_shader_calls == 1);
    expect("low_tier_material_gate_open", DAT_00865285 != 0);
    expect("low_tier_binds_material2_declaration_and_shader",
           set_vertex_declaration_calls == 1 && last_vertex_declaration == DAT_00cf1934 &&
               set_vertex_shader_calls == 1 && last_vertex_shader == DAT_00cf1924);
    expect("low_tier_pushed_transform_constant_register0",
           vs_const_calls_for_register[0] == 1 &&
               vs_const_data_for_register[0][0] == 1.0f && vs_const_data_for_register[0][1] == 0.0f &&
               vs_const_data_for_register[0][2] == 0.0f && vs_const_data_for_register[0][3] == 10.0f &&
               vs_const_data_for_register[0][7] == 20.0f && vs_const_data_for_register[0][11] == 30.0f &&
               vs_const_data_for_register[0][15] == 1.0f);

    // ---- low-tier path, with an index buffer: DrawIndexedPrimitive ---
    draw_primitive_calls = 0;
    *reinterpret_cast<std::uint32_t*>(item + 0x14) = 0; // index buffer handle 0 (real this time)
    *reinterpret_cast<std::uint16_t*>(item + 0x1c) = 3; // start index
    *reinterpret_cast<std::uint16_t*>(item + 0x26) = 9; // num vertices
    execute_scene_render_batch(context, item, 0, 0, nullptr);
    expect("low_tier_draws_indexed", draw_indexed_calls == 1 && draw_primitive_calls == 0);
    expect("low_tier_indexed_args",
           last_base_vertex == 7 && last_min_vertex == 0 && last_num_vertices == 9 &&
               last_start_index == 3 && last_indexed_primitive_count == 2);
    expect("low_tier_bound_indices", set_indices_calls == 1 && last_indices == fake_index_buffer_object);

    // ---- resolve_material_bindings: shared 2D-overlay material type ---
    // 0x20000108 is the same material hp6::render::render_fade_overlay_quad
    // already binds via DAT_00cf1930/DAT_00cf192c (ScreenOverlayRender.cpp)
    // -- confirms both real draw-call integrations in this project share
    // this real material.
    set_vertex_declaration_calls = 0;
    set_vertex_shader_calls = 0;
    *reinterpret_cast<std::uint32_t*>(item + 0x18) = 0x20000108u;
    DAT_00866360 = 0xffffffffu; // force the register-0x11 cache-miss branch too
    execute_scene_render_batch(context, item, 0, 0, nullptr);
    expect("low_tier_binds_overlay_material_declaration_and_shader",
           set_vertex_declaration_calls == 1 && last_vertex_declaration == DAT_00cf1930 &&
               set_vertex_shader_calls == 1 && last_vertex_shader == DAT_00cf192c);
    expect("low_tier_overlay_material_gate_open", DAT_00865285 != 0);
    expect("low_tier_overlay_material_still_draws", draw_indexed_calls == 2);

    // ---- resolve_material_bindings dispatches to
    // resolve_material_shader_effect for a material type outside the 3
    // fixed pairs above. With DAT_00ba14fc == 0, material type 0x101a
    // resolves to variant 0, group 0, index 2 -- cache slot 2, empty by
    // default, so the still-deferred FUN_00641f70 leaves a null
    // declaration/shader handle; the real bind calls still fire (with
    // that placeholder handle) and the draw gate stays open, matching
    // "only the shader bound is unfaithful, never whether the draw fires".
    set_vertex_declaration_calls = 0;
    set_vertex_shader_calls = 0;
    *reinterpret_cast<std::uint32_t*>(item + 0x18) = 0x101au;
    DAT_00866360 = 0xffffffffu;
    const int draw_indexed_calls_before_effect = draw_indexed_calls;
    execute_scene_render_batch(context, item, 0, 0, nullptr);
    expect("low_tier_shader_effect_binds_placeholder_handles",
           set_vertex_declaration_calls == 1 && last_vertex_declaration == nullptr &&
               set_vertex_shader_calls == 1 && last_vertex_shader == nullptr);
    expect("low_tier_shader_effect_gate_stays_open", DAT_00865285 != 0);
    expect("low_tier_shader_effect_still_draws", draw_indexed_calls == draw_indexed_calls_before_effect + 1);

    // ---- resolve_material_bindings: unrecognized material type closes
    // the draw gate (DAT_00865285), so the real draw call this project
    // integrates does NOT fire, matching the exported x86's own default
    // case (switchD_00644df9_caseD_3).
    *reinterpret_cast<std::uint32_t*>(item + 0x18) = 0x12345u; // not one of the recognized types
    const int draw_indexed_calls_before_unrecognized = draw_indexed_calls;
    execute_scene_render_batch(context, item, 0, 0, nullptr);
    expect("unrecognized_material_closes_gate", DAT_00865285 == 0);
    expect("unrecognized_material_skips_draw", draw_indexed_calls == draw_indexed_calls_before_unrecognized);

    // Restore a recognized material type before the remaining scenarios.
    *reinterpret_cast<std::uint32_t*>(item + 0x18) = 2;

    // ---- fuller path (DAT_008663d5 == 0): same real draw call --------
    DAT_008663d5 = 0;
    draw_indexed_calls = 0;
    set_indices_calls = 0;
    DAT_0086635c = 0xffffffffu; // force the cache-miss branch
    execute_scene_render_batch(context, item, 0, 0, nullptr);
    expect("full_tier_draws_indexed", draw_indexed_calls == 1);
    expect("full_tier_bound_indices", set_indices_calls == 1);
    expect("full_tier_caches_material_flags_byte8", DAT_00866360 == 0); // item+8 left zeroed

    // ---- 'M' multi-item batch recurses and advances the context ------
    draw_primitive_calls = 0;
    draw_indexed_calls = 0;
    DAT_008663d5 = 1; // back to the low-tier path for a simple, countable draw
    std::uint8_t multi[0x10 + 8 * 2] = {};
    multi[0] = 'M';
    *reinterpret_cast<std::uint16_t*>(multi + 2) = 2; // two entries
    std::uint8_t sub_item[0x28] = {};
    *reinterpret_cast<std::uint32_t*>(sub_item + 0x10) = 0xbad1bad1u; // no vertex buffer: skipped
    *reinterpret_cast<std::uint32_t*>(sub_item + 0x14) = 0xbad1bad1u;
    *reinterpret_cast<void**>(multi + 0x10) = &item;   // entry 0: the real low-tier item above
    *reinterpret_cast<std::uint32_t*>(multi + 0x14) = 0;
    *reinterpret_cast<void**>(multi + 0x18) = sub_item; // entry 1: a no-op (no vertex buffer)
    *reinterpret_cast<std::uint32_t*>(multi + 0x1c) = 0;

    const std::int32_t context_before = context[0];
    execute_scene_render_batch(context, multi, 0, 0, nullptr);
    expect("multi_batch_recursed_into_real_item", draw_indexed_calls == 1);
    expect("multi_batch_advanced_context_cursor", context[0] == context_before + 0x80);

    // ---- flags bit 0x80000000: invalidate path, no draw --------------
    draw_primitive_calls = 0;
    draw_indexed_calls = 0;
    std::uint8_t region_entry[0x1c + 8 * 0xc] = {};
    std::uint8_t chained_node[0x28] = {};
    *reinterpret_cast<std::uint16_t*>(chained_node + 0x26) = 0x0001; // starts clear
    *reinterpret_cast<std::uint8_t**>(region_entry + 0x1c) = chained_node;
    *reinterpret_cast<std::uint32_t*>(region_entry + 0x14) = 1; // bit 0 pending
    DAT_00cf18ec = 42;
    execute_scene_render_batch(context, nullptr, 0x80000000u, 0, region_entry);
    expect("invalidate_marks_chained_node_dirty",
           (*reinterpret_cast<std::uint16_t*>(chained_node + 0x26) & 0x4000u) != 0);
    expect("invalidate_clears_region_material_cache", DAT_00cf18ec == 0);
    expect("invalidate_does_not_draw", draw_primitive_calls == 0 && draw_indexed_calls == 0);

    if (failures != 0) {
        std::printf("scene_batch_renderer_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("scene_batch_renderer_smoke: all checks passed\n");
    return 0;
}
