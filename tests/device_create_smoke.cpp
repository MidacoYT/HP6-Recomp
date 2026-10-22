// Local behavior checks for the Direct3D 9 device-creation chain
// (FUN_006498e0, reached from WinMain via FUN_005dca10 — see
// architecture/startup_path.md). Game-space symbols and fake
// IDirect3D9/IDirect3DDevice9-shaped vtables get test-local definitions.
#include "render/DeviceCreate.h"
#include "render/DeviceReset.h"
#include "render/RenderStateCache.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#define HP6_STDCALL __stdcall
#else
#define HP6_STDCALL
#endif

// ---- hp6::platform::service_holder dependencies (also required by
// RenderHeap.cpp's initialize_render_framework_heap) ------------------------
void* PTR_FUN_0083e55c[27] = {};
void* DAT_00fcdd74[6] = {};
void** DAT_00cef6bc = nullptr;
std::uint32_t DAT_00fd1254 = 0;
void cleanup_00711e60() {}
void* PTR_FUN_007159b0 = nullptr;
extern void* PTR_FUN_00715d0c[1] = {};
extern void* PTR_LAB_00715da8[26] = {};
void* PTR_LAB_0071b520 = nullptr;
std::uint32_t DAT_008998c8 = 0;

// ---- hp6::runtime::FrameClock dependencies (pulled in by
// reset_frame_pump_state) ---------------------------------------------------
void* DAT_00cef730 = nullptr;
std::uint8_t DAT_00cef72c = 0;
float DAT_00ba9504 = 0.0f;
float DAT_008482ec = 65536.0f;
void* DAT_0089eb0c = nullptr;
void* DAT_00fc7f90 = nullptr;
std::int32_t DAT_0089eb14 = 0;
std::int32_t DAT_00cef790 = 0;
std::uint32_t DAT_00d64570 = 0;
std::int32_t DAT_00d64574 = 0;
std::uint32_t DAT_00d64578 = 0;
std::int32_t DAT_00d6457c = 0;
std::int32_t DAT_00d644f0 = 0;
std::int32_t DAT_00d644f4 = 0;
float DAT_0080c680 = 1.0f;
float DAT_0083b230 = 1.0f;
float DAT_008003ac = 1.0f;

// ---- RenderHeap.cpp's extern dependencies ---------------------------------
std::uint32_t DAT_008796b0 = 0;
std::uint8_t* PTR_DAT_008796b4 = nullptr;
std::uint32_t DAT_008796b8 = 0;
std::uint8_t* PTR_DAT_008796bc = nullptr;
std::uint32_t DAT_008796c8 = 0;
std::uint8_t* PTR_DAT_008796cc = nullptr;

// ---- DeviceCreate.cpp's create_and_configure_render_device dependencies --
std::uint8_t DAT_00d71250[0x268] = {};
std::uint32_t DAT_00c75538[14] = {};
void* DAT_00d6af14 = nullptr;

// ---- game-space globals (DeviceCreate.cpp's extern dependencies) ---------
void* DAT_00cf194c = nullptr;
std::int32_t DAT_00cf19d0 = 0;
std::uint32_t DAT_00cf19c8 = 0;
std::int32_t DAT_008663d8 = 0;
void* DAT_00cf1944 = nullptr;
std::uint32_t DAT_00cf19d4 = 0;
std::uint8_t DAT_00865286 = 0;
void* DAT_00c75a7c = nullptr;
std::int32_t DAT_00cf19bc = 0;
std::uint32_t _DAT_00cf19c0 = 0;
std::uint32_t DAT_00cf1964 = 0;
std::uint8_t DAT_008663d5 = 0;
std::uint32_t DAT_008663dc = 0;
std::uint32_t DAT_00866364 = 0;

std::uint32_t DAT_009978e4 = 0x1234; // fake HWND
void* DAT_00cef694 = reinterpret_cast<void*>(0x1234);
std::int32_t DAT_00cf1950 = 0;
std::uint32_t DAT_00cf19a8 = 0;
std::int32_t DAT_00cf198c = 0;
std::int32_t DAT_00cf1990 = 0;
std::uint32_t DAT_00cf19cc = 0;
std::uint32_t DAT_00b170b8 = 0;
float DAT_008663ac = 0.0f;
std::uint32_t _DAT_00b10894 = 0;
float _DAT_008a4474 = 0.0f;
float DAT_0084859c = 1.0f;

std::uint32_t DAT_008663b0 = 0;
std::uint32_t DAT_008663b4 = 0;
std::int32_t DAT_00cf1998 = 0;
void* DAT_00bb0a40 = nullptr;
void* DAT_00ba14f8 = nullptr;
void* DAT_00cf1948 = nullptr;
void* DAT_00cf1954 = nullptr;
void* DAT_00cf195c = nullptr;
void* DAT_00cf1958 = nullptr;
void* DAT_00cf1960 = nullptr;
std::uint8_t DAT_008663d4 = 1;
std::uint8_t DAT_00865287 = 1;

std::int32_t _DAT_00cf19ac = 0;
std::int32_t _DAT_00cf19b0 = 0;
std::int32_t _DAT_00cf19b4 = 0;
std::int32_t _DAT_00cf19b8 = 0;
float DAT_00805f14 = 1.0f;
float _DAT_00848588 = 1.0f;

std::uint32_t DAT_00cf19e8 = 0;
std::uint32_t DAT_00cef794 = 0;
std::uint32_t DAT_00866a74 = 0;
std::uint32_t DAT_008663c4 = 0;
std::uint32_t DAT_00848598 = 0;
std::uint32_t DAT_00834bb0 = 0;
std::uint32_t DAT_00848594 = 0;
std::uint32_t _DAT_0082cf24 = 0;
std::uint32_t _DAT_0082d600 = 0;
std::uint32_t DAT_00848590 = 0;
std::uint32_t DAT_0084858c = 0;
std::uint8_t DAT_00cf19dd = 0;

std::int32_t DAT_00cf1970 = 0;
std::int32_t DAT_00cf1978 = 0;
std::uint32_t DAT_008663e0 = 0;
std::int32_t DAT_00cf1980 = 0;
float _DAT_0083b230 = 1.0f;
std::int32_t DAT_00cf1988 = 0;
std::int32_t DAT_00cf1984 = 0;
float _DAT_0082ab5c = 1.0f;
std::int32_t DAT_00cf197c = 0;

std::uint32_t DAT_008663c8 = 0;

struct ShaderSlot { std::uint32_t bytecode; std::uint32_t shader; };
ShaderSlot DAT_00a68820[4] = {};
std::uint32_t DAT_00cf1a30 = 0;
std::uint32_t DAT_00cf1a38 = 0;
std::uint8_t DAT_00f38b10[0x600 * 0xc4] = {};
std::int32_t DAT_008928c0[256] = {};
std::int32_t DAT_008663b8 = 0;
std::int32_t DAT_00cf1974 = 0;

std::uint8_t DAT_00b108a4[8 * 0x80] = {};
std::uint8_t DAT_009968dc[8 * 0x80] = {};
std::uint8_t DAT_00c751a4[0x320] = {};
std::uint32_t DAT_00bb09b4[16] = {};

// DeviceReset.cpp / RenderStateCache.cpp / RenderTargetSetup.cpp externs
// (not all exercised here, but required to link this binary).
std::uint32_t DAT_00c75188[256] = {};
std::uint32_t DAT_009968d8[256] = {};

extern "C" void* HP6_STDCALL Direct3DCreate9(std::uint32_t) {
    return nullptr; // never actually reached: tests pre-set DAT_00cf194c
}

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}
std::uint32_t address(const void* value) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}

void* device_vtable[0x120 / sizeof(void*) + 1] = {};
struct FakeDevice { void* vtable = device_vtable; };
FakeDevice fake_device;
void* fake_device_instance = &fake_device;

void* swap_chain_vtable[8] = {};
struct FakeSwapChain { void* vtable = swap_chain_vtable; };
FakeSwapChain fake_swap_chain;
void* fake_swap_chain_instance = &fake_swap_chain;

void* surface_vtable[16] = {};
struct FakeSurface { void* vtable = surface_vtable; };
FakeSurface fake_surface;
void* fake_surface_instance = &fake_surface;

void* depth_surface_vtable[16] = {};
struct FakeDepthSurface { void* vtable = depth_surface_vtable; };
FakeDepthSurface fake_depth_surface;
void* fake_depth_surface_instance = &fake_depth_surface;

void* texture_vtable[20] = {};
struct FakeTexture { void* vtable = texture_vtable; };
FakeTexture fake_texture;
void* fake_texture_instance = &fake_texture;

// ---- fake IDirect3D9 -----------------------------------------------------
std::uint32_t HP6_STDCALL fake_get_adapter_count(void*) { return 1; }
std::int32_t HP6_STDCALL fake_get_adapter_identifier(void*, std::uint32_t, std::uint32_t, void*) {
    return 0;
}
std::int32_t HP6_STDCALL fake_get_device_caps(void*, std::uint32_t, std::uint32_t, void* out) {
    std::memset(out, 0, 0x130); // remaining space in the 0x584-byte table entry
    *reinterpret_cast<std::int32_t*>(out) = 1; // D3DDEVTYPE_HAL
    return 0;
}
std::uint32_t HP6_STDCALL fake_get_adapter_mode_count(void*, std::uint32_t, std::uint32_t format) {
    return (format == 0x17) ? 1u : 0u; // only expose one X8R8G8B8 mode
}
std::int32_t HP6_STDCALL fake_enum_adapter_modes(void*, std::uint32_t, std::uint32_t format,
                                                 std::uint32_t, void* out_mode) {
    auto* mode = reinterpret_cast<std::uint32_t*>(out_mode);
    mode[0] = 640;
    mode[1] = 480;
    mode[2] = 60;
    mode[3] = format;
    return 0;
}
std::int32_t HP6_STDCALL fake_get_adapter_display_mode(void*, std::uint32_t, void* out_mode) {
    auto* mode = reinterpret_cast<std::uint32_t*>(out_mode);
    mode[0] = 640;
    mode[1] = 480;
    mode[2] = 60;
    mode[3] = 0x16; // D3DFMT_R5G6B5
    return 0;
}
int check_device_type_calls = 0;
std::int32_t HP6_STDCALL fake_check_device_type(void*, std::uint32_t, std::uint32_t, std::uint32_t,
                                                std::uint32_t, std::uint32_t) {
    ++check_device_type_calls;
    return 0;
}
int create_device_calls = 0;
void* last_present_params = nullptr;
std::int32_t HP6_STDCALL fake_create_device(void*, std::uint32_t, std::uint32_t,
                                            std::uint32_t hwnd, std::uint32_t, void* params,
                                            void* out_device) {
    ++create_device_calls;
    last_present_params = params;
    (void)hwnd;
    *reinterpret_cast<void**>(out_device) = fake_device_instance;
    return 0;
}

void* d3d9_vtable[20] = {};
struct FakeD3D9 {
    void* vtable = d3d9_vtable;
};
FakeD3D9 fake_d3d9;

// ---- fake IDirect3DDevice9 ------------------------------------------------
int release_calls = 0;
void HP6_STDCALL fake_release(void*) { ++release_calls; }

std::uint32_t available_texture_mem_bytes = 16u << 20;
std::uint32_t HP6_STDCALL fake_get_available_texture_mem(void*) {
    return available_texture_mem_bytes;
}

int get_swap_chain_calls = 0;
std::int32_t HP6_STDCALL fake_get_swap_chain(void*, std::uint32_t, void* out) {
    ++get_swap_chain_calls;
    *reinterpret_cast<void**>(out) = fake_swap_chain_instance;
    return 0;
}

int get_back_buffer_calls = 0;
std::int32_t HP6_STDCALL fake_get_back_buffer(void*, std::uint32_t, std::uint32_t, void* out) {
    ++get_back_buffer_calls;
    *reinterpret_cast<void**>(out) = fake_surface_instance;
    return 0;
}

int get_desc_calls = 0;
std::int32_t HP6_STDCALL fake_get_desc(void*, void* out) {
    ++get_desc_calls;
    auto* desc = reinterpret_cast<std::uint32_t*>(out);
    desc[0] = 0x15; // Format
    desc[6] = 640;  // Width
    desc[7] = 480;  // Height
    return 0;
}

int clear_calls = 0;
std::int32_t HP6_STDCALL fake_clear(void*, std::uint32_t, void*, std::uint32_t, std::uint32_t,
                                    float, std::uint32_t) {
    ++clear_calls;
    return 0;
}
int present_calls = 0;
std::int32_t HP6_STDCALL fake_present(void*, void*, void*, void*, void*) {
    ++present_calls;
    return 0;
}

int create_depth_stencil_calls = 0;
std::int32_t HP6_STDCALL fake_create_depth_stencil(void*, std::uint32_t, std::uint32_t,
                                                   std::uint32_t, std::uint32_t, std::uint32_t,
                                                   std::uint32_t, void* out, std::uint32_t) {
    ++create_depth_stencil_calls;
    *reinterpret_cast<void**>(out) = fake_depth_surface_instance;
    return 0;
}

int create_texture_calls = 0;
std::int32_t HP6_STDCALL fake_create_texture(void*, std::uint32_t, std::uint32_t, std::uint32_t,
                                             std::uint32_t, std::uint32_t, std::uint32_t,
                                             void* out, std::uint32_t) {
    ++create_texture_calls;
    *reinterpret_cast<void**>(out) = fake_texture_instance;
    return 0;
}

int get_surface_level_calls = 0;
std::int32_t HP6_STDCALL fake_get_surface_level(void*, std::uint32_t, void* out) {
    ++get_surface_level_calls;
    *reinterpret_cast<void**>(out) = fake_surface_instance;
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
int set_render_state_calls = 0;
std::int32_t HP6_STDCALL fake_set_render_state(void*, std::uint32_t, std::uint32_t) {
    ++set_render_state_calls;
    return 0;
}
int set_sampler_state_calls = 0;
std::int32_t HP6_STDCALL fake_set_sampler_state(void*, std::uint32_t, std::uint32_t, std::uint32_t) {
    ++set_sampler_state_calls;
    return 0;
}
int set_texture_stage_state_calls = 0;
std::int32_t HP6_STDCALL fake_set_texture_stage_state(void*, std::uint32_t, std::uint32_t,
                                                       std::uint32_t) {
    ++set_texture_stage_state_calls;
    return 0;
}
int create_vb_calls = 0;
std::int32_t HP6_STDCALL fake_create_vertex_buffer(void*, std::uint32_t, std::uint32_t,
                                                   std::uint32_t, std::uint32_t, void* out,
                                                   std::uint32_t) {
    ++create_vb_calls;
    *reinterpret_cast<void**>(out) = reinterpret_cast<void*>(0x5000);
    return 0;
}

int create_heap_calls = 0;
std::int32_t last_heap_size = 0;
void* HP6_STDCALL fake_create_heap(std::int32_t size, const void*) {
    ++create_heap_calls;
    last_heap_size = size;
    return reinterpret_cast<void*>(0x9000);
}

} // namespace

int main() {
    using namespace hp6::render;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    d3d9_vtable[0x10 / 4] = reinterpret_cast<void*>(&fake_get_adapter_count);
    d3d9_vtable[0x14 / 4] = reinterpret_cast<void*>(&fake_get_adapter_identifier);
    d3d9_vtable[0x38 / 4] = reinterpret_cast<void*>(&fake_get_device_caps);
    d3d9_vtable[0x18 / 4] = reinterpret_cast<void*>(&fake_get_adapter_mode_count);
    d3d9_vtable[0x1c / 4] = reinterpret_cast<void*>(&fake_enum_adapter_modes);
    d3d9_vtable[0x20 / 4] = reinterpret_cast<void*>(&fake_get_adapter_display_mode);
    d3d9_vtable[0x24 / 4] = reinterpret_cast<void*>(&fake_check_device_type);
    d3d9_vtable[0x40 / 4] = reinterpret_cast<void*>(&fake_create_device);
    DAT_00cf194c = &fake_d3d9;

    device_vtable[0x10 / 4] = reinterpret_cast<void*>(&fake_get_available_texture_mem);
    device_vtable[0x38 / 4] = reinterpret_cast<void*>(&fake_get_swap_chain);
    device_vtable[0xac / 4] = reinterpret_cast<void*>(&fake_clear);
    device_vtable[0x44 / 4] = reinterpret_cast<void*>(&fake_present);
    device_vtable[0x74 / 4] = reinterpret_cast<void*>(&fake_create_depth_stencil);
    device_vtable[0x5c / 4] = reinterpret_cast<void*>(&fake_create_texture);
    device_vtable[0x94 / 4] = reinterpret_cast<void*>(&fake_set_render_target);
    device_vtable[0x9c / 4] = reinterpret_cast<void*>(&fake_set_depth_stencil);
    device_vtable[0xe4 / 4] = reinterpret_cast<void*>(&fake_set_render_state);
    device_vtable[0x114 / 4] = reinterpret_cast<void*>(&fake_set_sampler_state);
    device_vtable[0x10c / 4] = reinterpret_cast<void*>(&fake_set_texture_stage_state);
    device_vtable[0x68 / 4] = reinterpret_cast<void*>(&fake_create_vertex_buffer);
    device_vtable[8 / 4] = reinterpret_cast<void*>(&fake_release);

    swap_chain_vtable[0x14 / 4] = reinterpret_cast<void*>(&fake_get_back_buffer);
    surface_vtable[0x30 / 4] = reinterpret_cast<void*>(&fake_get_desc);
    surface_vtable[8 / 4] = reinterpret_cast<void*>(&fake_release);
    depth_surface_vtable[0x30 / 4] = reinterpret_cast<void*>(&fake_get_desc);
    depth_surface_vtable[8 / 4] = reinterpret_cast<void*>(&fake_release);
    texture_vtable[0x48 / 4] = reinterpret_cast<void*>(&fake_get_surface_level);
    texture_vtable[8 / 4] = reinterpret_cast<void*>(&fake_release);

    // ---- enumerate_display_modes (00649140) --------------------------------
    DAT_00cf19d0 = 0;
    std::int32_t rc = enumerate_display_modes();
    expect("enum_modes_ok", rc == 0);
    expect("enum_modes_table_built", DAT_00cf19d0 != 0);
    expect("enum_modes_adapter_count", DAT_00cf19c8 == 1);
    rc = enumerate_display_modes();
    expect("enum_modes_idempotent", rc == 0);

    // ---- create_d3d9_device (006498e0) -------------------------------------
    DAT_00cf19d0 = 0; // rebuild inside create_d3d9_device too
    DAT_008663d8 = 0;
    DAT_00cf1950 = 0; // default/windowed path, not the deferred custom-resolution branch
    create_device_calls = 0;
    check_device_type_calls = 0;
    clear_calls = 0;
    present_calls = 0;
    create_depth_stencil_calls = 0;
    create_vb_calls = 0;

    const std::int32_t result = create_d3d9_device(640, 480, 0, 0, 6);
    expect("create_device_result_ok", result == 0);
    expect("create_device_called", create_device_calls == 1);
    expect("check_device_type_called", check_device_type_calls >= 1);
    expect("device_assigned", DAT_00cf1944 == fake_device_instance);
    expect("swap_chain_fetched", get_swap_chain_calls >= 1);
    expect("back_buffer_fetched", get_back_buffer_calls >= 1);
    expect("depth_stencil_created", create_depth_stencil_calls >= 1);
    expect("render_target_bound", set_render_target_calls >= 1);
    expect("depth_stencil_bound", set_depth_stencil_calls >= 1);
    expect("cleared_and_presented", clear_calls >= 2 && present_calls >= 2);
    expect("vertex_buffer_created", create_vb_calls >= 1);
    expect("render_states_applied", set_render_state_calls > 0);
    expect("sampler_states_applied", set_sampler_state_calls > 0);
    expect("texture_stage_states_applied", set_texture_stage_state_calls > 0);
    expect("saved_hwnd_used", last_present_params != nullptr);

    // ---- create_and_configure_render_device (005dca10) --------------------
    PTR_FUN_0083e55c[4] = reinterpret_cast<void*>(&fake_create_heap);
    DAT_00cf194c = &fake_d3d9;
    DAT_00cf1944 = nullptr;
    DAT_00cf19d0 = 0;
    DAT_008663d8 = 0;
    create_device_calls = 0;
    create_heap_calls = 0;
    std::memset(DAT_00d71250, 0, sizeof DAT_00d71250);

    const std::int32_t top_result = create_and_configure_render_device(640, 480);
    expect("top_level_result_ok", top_result == 0);
    expect("top_level_copied_game_window", DAT_009978e4 == 0x1234);
    expect("heap_initialized", DAT_00fd1254 & 1u);
    expect("heap_created", create_heap_calls == 1);
    expect("heap_size_has_two_regions_overhead", last_heap_size == 0x2000);
    expect("heap_handle_stored",
           *reinterpret_cast<void**>(DAT_00d71250 + 0x260) == reinterpret_cast<void*>(0x9000));
    expect("top_level_created_device", create_device_calls == 1);
    expect("top_level_device_assigned", DAT_00cf1944 == fake_device_instance);

    if (failures != 0) {
        std::printf("device_create_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("device_create_smoke: all checks passed\n");
    return 0;
}
