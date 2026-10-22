// End-to-end smoke test for FUN_005d9300, the game's real WinMain,
// assembled in src/platform/WinMain.cpp from every piece this project
// has reconstructed and tested in isolation (device creation, engine
// bootstrap, TriggerBootupFlow, the message loop, device-lost/shutdown
// handling). This test fakes the Direct3D 9 object chain (the same way
// tests/device_create_smoke.cpp and tests/device_lost_handler_smoke.cpp
// already do) and pre-arms the message loop's one-shot quit guard
// (DAT_00cef68d) so run_message_loop returns immediately instead of
// looping forever waiting for a real Windows message.
#include "platform/EngineSubsystems.h"
#include "platform/WinMain.h"
#include "presentation/BootupFlow.h"
#include "presentation/ChapterFlow.h"
#include "presentation/CommandDispatch.h"
#include "presentation/TaskSystem.h"
#include "render/DeviceCreate.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifdef _MSC_VER
#define HP6_STDCALL __stdcall
#else
#define HP6_STDCALL
#endif

// ---- hp6::platform::service_holder dependencies ---------------------------
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

// ---- hp6::runtime::FrameClock dependencies ---------------------------------
void* DAT_00cef730 = nullptr;
std::uint8_t DAT_00cef72c = 0;
float DAT_00ba9504 = 1000.0f;
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

// ---- MessageLoop.cpp's own extern dependencies -----------------------------
std::uint8_t DAT_00cef68d = 0; // WM_QUIT-seen guard; pre-set below to avoid hanging
std::uint8_t DAT_00866e2a = 0; // "engine is actually running" flag
std::uint8_t DAT_00cef68f = 0;
std::int32_t DAT_00cef55c = 0;
std::int32_t DAT_00866e24 = 0;
std::int32_t DAT_00cef6a0 = 0;
std::uint8_t DAT_00cef69c = 1;
std::uint8_t DAT_00cef69d = 1;
void* DAT_00fcade0 = nullptr;
std::uint8_t DAT_00cef69e = 0;

// ---- hp6::presentation::initialize_engine_core_systems's own extern
// dependencies ---------------------------------------------------------------
std::int32_t DAT_00d63eb0 = 0, DAT_00d63df8 = 0, DAT_00d63e30 = 0;
std::int32_t DAT_00d63ea0 = 0, DAT_00d63de4 = 0;
std::int32_t DAT_00d63ec0 = 0, DAT_00d63eb8 = 0, DAT_00d63e68 = 0, DAT_00d63ed0 = 0,
             DAT_00d63e58 = 0;
std::int32_t DAT_00d63e88 = 0, DAT_00d63e38 = 0, DAT_00d63e28 = 0, DAT_00d63ec8 = 0,
             DAT_00d63e90 = 0;
std::int32_t DAT_00d63e50 = 0, DAT_00d63ea8 = 0, DAT_00d63e40 = 0, DAT_00d63e18 = 0,
             DAT_00d63e98 = 0;
std::int32_t DAT_00d63e10 = 0, DAT_00d63e00 = 0, DAT_00d63e08 = 0;
std::int32_t DAT_00d63e70 = 0, DAT_00d63ed8 = 0, DAT_00d63e60 = 0;
std::int32_t DAT_00d63e80 = 0; // "not registered" sentinel
std::uint32_t DAT_00cef884 = 0, DAT_00cef880 = 0;
std::uint32_t DAT_00fc8478 = 0;
void* PTR_PTR_008813c8 = nullptr;
std::uint8_t DAT_0087cc00[8] = {};
void* PTR_PTR_0087eadc = nullptr;
void* DAT_00d6af14 = nullptr;
std::uint8_t DAT_00cef718 = 0, DAT_00cef69f = 0;
float _DAT_008a4474 = 60.0f;

// ---- ChapterFlow.cpp's own extern dependencies -----------------------------
std::uint32_t DAT_00fd1b14 = 0;
void* DAT_00fcd530 = nullptr;
std::uint8_t* DAT_00fcd534 = nullptr;
std::int32_t DAT_00d472dc = -1;
std::uint8_t DAT_0087e014 = 1;
std::uint32_t DAT_00fd178c = 0;
void* DAT_00fd1718 = nullptr;
struct ChapterTableEntry {
    std::int32_t index;
    const char* name;
    std::int32_t next_hint;
};
ChapterTableEntry DAT_00892300[10] = {
    {9, "LanguageSelect", 0}, {8, "FrontEnd", 0},       {1, "BurrowIntro", 2},
    {2, "Hogwarts", 3},       {3, "BurrowXmas", 4},     {4, "Hogwarts", 5},
    {5, "TheCave", 6},        {6, "Hogwarts", 7},       {7, "Hogwarts", 8},
    {10, "Hogwarts", 0},
};
const char* PTR_DAT_0080a91c[11] = {
    "NONE",        "BURROW_INTRO", "HOGWARTS_1",    "BURROW_XMAS", "HOGWARTS_2",
    "THE_CAVE",    "HOGWARTS_3",   "ENDLESS_DAY",   "FRONT_END",   "LANGUAGE_SELECT",
    "POTIONS_GAME",
};

// ---- handle_trigger_bootup_flow's own extern dependencies ------------------
void* DAT_00fcadf0 = nullptr; // PresentationTaskHandler
extern void* PTR_LAB_00828360[4] = {}; // test-local task vtable slots
void* DAT_00fcadec = nullptr;
std::uint8_t DAT_008663d5 = 1; // quality tier already resolved

// ---- RenderHeap.cpp's extern dependencies ----------------------------------
std::uint32_t DAT_008796b0 = 0;
std::uint8_t* PTR_DAT_008796b4 = nullptr;
std::uint32_t DAT_008796b8 = 0;
std::uint8_t* PTR_DAT_008796bc = nullptr;
std::uint32_t DAT_008796c8 = 0;
std::uint8_t* PTR_DAT_008796cc = nullptr;

// ---- DeviceCreate.cpp's create_and_configure_render_device dependencies ---
std::uint8_t DAT_00d71250[0x268] = {};
std::uint32_t DAT_00c75538[14] = {};

// ---- game-space globals (DeviceCreate.cpp's own extern dependencies) ------
void* DAT_00cf194c = nullptr;
std::int32_t DAT_00cf19d0 = 0;
std::uint32_t DAT_00cf19c8 = 0;
std::int32_t DAT_008663d8 = 0;
void* DAT_00cf1944 = nullptr;
std::uint32_t DAT_00cf19d4 = 0;
std::int32_t DAT_00b108a0[256] = {};
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
std::uint8_t DAT_00865286 = 0;
void* DAT_00c75a7c = nullptr;
std::int32_t DAT_00cf19bc = 0;
std::uint32_t _DAT_00cf19c0 = 0;
std::uint32_t DAT_00cf1964 = 0;
std::uint32_t DAT_008663dc = 0;
std::uint32_t DAT_00866364 = 0;
std::int32_t DAT_00cf1950 = 0;
std::uint32_t DAT_00cf19a8 = 0;
std::int32_t DAT_00cf198c = 0;
std::int32_t DAT_00cf1990 = 0;
std::uint32_t DAT_00cf19cc = 0;
std::uint32_t DAT_00b170b8 = 0;
float DAT_008663ac = 0.0f;
std::uint32_t _DAT_00b10894 = 0;
float DAT_0084859c = 1.0f;
std::uint32_t DAT_008663b0 = 0;
std::uint32_t DAT_008663b4 = 0;
std::int32_t DAT_00cf1998 = 1; // render-to-texture disabled, matching device_lost_handler_smoke
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
std::int32_t DAT_00c75188[256] = {};
std::int32_t DAT_009968d8[256 * 0x20] = {};

// ---- handle_device_lost / shutdown_render_device's own extern deps --------
std::uint8_t DAT_00cf19dc = 0;
std::uint8_t DAT_00cf18c7 = 0;
std::int32_t DAT_00cf1938 = 0;
std::uint8_t DAT_00cf19df = 0;
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
std::int32_t DAT_0086527c = 0, DAT_00865280 = 0;
void* DAT_00cf19d8 = nullptr;

// ---- RegistryConfig.cpp's own extern dependencies --------------------------
std::int32_t DAT_00cf19a4 = 0;
std::int32_t DAT_008663bc = -1;
std::int32_t DAT_008663c0 = 0;

// ---- StorageInit.cpp's own extern dependencies -----------------------------
std::int32_t DAT_00851380[0x50 / 4] = {};
std::uint32_t DAT_00892b80[0x68 / 4] = {};
extern const std::uint32_t PTR_DAT_008513e0[26] = {};
std::uint8_t DAT_00ced2b0 = 0;
std::uint8_t DAT_00cf1b04 = 0;
std::uint8_t DAT_00853f84 = 0;
std::int32_t DAT_00cf1b00 = 0;
std::uint8_t DAT_00892df0 = 0; // reset_particle_system_tables's one-shot guard
char DAT_00d63f68[0x200] = {};   // parse_command_line_overrides's own copy of the command line
char DAT_00d64168[0x200] = {};   // a second copy the real call site also makes
char* DAT_00d64368[0x20] = {};   // "-flag" plain overrides
char* DAT_00d643e8[0x20] = {};   // "-key=value" keys
char* DAT_00d64468[0x20] = {};   // "-key=value" values
std::int32_t DAT_00d644e8 = 0;   // plain-override count
std::int32_t DAT_00d644ec = 0;   // pair-override count
std::uint32_t DAT_009978e4 = 0; // saved HWND for CreateDevice's hFocusWindow; never
                                 // actually produced by this project's own WinMain
                                 // reconstruction either (see DeviceCreate.cpp/
                                 // GameWindow.cpp doc comments) -- a test-local default,
                                 // the same way tests/device_create_smoke.cpp uses one.

namespace hp6::platform {
std::int32_t register_timer_completion(void*, void*, std::int32_t, std::int32_t, std::uint32_t,
                                       std::int32_t) {
    return 0;
}
} // namespace hp6::platform

// ---- WinMain.cpp's own extern dependencies ---------------------------------
std::int32_t DAT_00cf1968 = 0;
std::int32_t DAT_00cf196c = 0;
std::int32_t DAT_00cf1994 = 0;
std::int32_t DAT_00cf199c = 0;
std::int32_t DAT_0096b468[6] = {};
std::int32_t DAT_00a70860[6] = {};
float DAT_008663a8 = 0.0f;
float DAT_007289d4 = 1.3333f;
void* DAT_00cef694 = nullptr;
void* DAT_00cef698 = nullptr;
std::uint8_t DAT_00cef68e = 0;
std::uint8_t DAT_0089c427 = 0;

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

// ---- fake nested allocator shared by allocate_via_task_handler /
// submit_task, matching tests/bootup_flow_smoke.cpp's own pattern --------
std::uint8_t node_arena[4][0x20];
int next_node = 0;
void* HP6_STDCALL fake_node_allocate(std::int32_t, const void*) {
    void* slot = node_arena[next_node];
    ++next_node;
    return slot;
}
void* node_allocator_vtable[1] = {reinterpret_cast<void*>(&fake_node_allocate)};
struct FakeNodeAllocator { void* vtable = node_allocator_vtable; };
FakeNodeAllocator fake_node_allocator;

struct FakeTaskHandler {
    std::uint8_t pad[8] = {};
    void* allocator = &fake_node_allocator;
    void* queue_head = nullptr;
};
FakeTaskHandler fake_task_handler;

std::int32_t HP6_STDCALL fake_bootup_task_ready() { return 0; }
void* bootup_task_vtable[2] = {nullptr, reinterpret_cast<void*>(&fake_bootup_task_ready)};

std::int32_t unused_command_handler(const char*) { return 0; }

// ---- generic heap-factory, backing both RenderHeap's
// initialize_render_framework_heap and the engine bootstrap's time
// manager allocation (the same service-holder slot serves both) -------
void* HP6_STDCALL fake_create_heap(std::int32_t size, const void*) {
    return std::malloc(static_cast<std::size_t>(size));
}

// ---- fake Direct3D9 object chain (union of tests/device_create_smoke.cpp
// and tests/device_lost_handler_smoke.cpp's own fakes, since WinMain
// exercises both device creation AND device-lost/shutdown handling) ------
void* device_vtable[0x120 / sizeof(void*) + 1] = {};
struct FakeDevice { void* vtable = device_vtable; };
FakeDevice fake_device;

void* swap_chain_vtable[8] = {};
struct FakeSwapChain { void* vtable = swap_chain_vtable; };
FakeSwapChain fake_swap_chain;

void* surface_vtable[16] = {};
struct FakeSurface { void* vtable = surface_vtable; };
FakeSurface fake_surface;

void* depth_surface_vtable[16] = {};
struct FakeDepthSurface { void* vtable = depth_surface_vtable; };
FakeDepthSurface fake_depth_surface;

void* texture_vtable[20] = {};
struct FakeTexture { void* vtable = texture_vtable; };
FakeTexture fake_texture;

void* vertex_buffer_vtable[4] = {};
struct FakeVertexBuffer { void* vtable = vertex_buffer_vtable; };
FakeVertexBuffer fake_vertex_buffer;

void* d3d9_vtable[20] = {};
struct FakeD3D9 { void* vtable = d3d9_vtable; };
FakeD3D9 fake_d3d9;

void HP6_STDCALL fake_release(void*) {}

std::uint32_t HP6_STDCALL fake_get_adapter_count(void*) { return 1; }
std::int32_t HP6_STDCALL fake_get_adapter_identifier(void*, std::uint32_t, std::uint32_t, void*) {
    return 0;
}
std::int32_t HP6_STDCALL fake_get_device_caps(void*, std::uint32_t, std::uint32_t, void* out) {
    std::memset(out, 0, 0x130);
    *reinterpret_cast<std::int32_t*>(out) = 1; // D3DDEVTYPE_HAL
    return 0;
}
std::uint32_t HP6_STDCALL fake_get_adapter_mode_count(void*, std::uint32_t, std::uint32_t format) {
    return (format == 0x17) ? 1u : 0u;
}
std::int32_t HP6_STDCALL fake_enum_adapter_modes(void*, std::uint32_t, std::uint32_t format,
                                                 std::uint32_t, void* out_mode) {
    auto* mode = reinterpret_cast<std::uint32_t*>(out_mode);
    mode[0] = 640; mode[1] = 480; mode[2] = 60; mode[3] = format;
    return 0;
}
std::int32_t HP6_STDCALL fake_get_adapter_display_mode(void*, std::uint32_t, void* out_mode) {
    auto* mode = reinterpret_cast<std::uint32_t*>(out_mode);
    mode[0] = 640; mode[1] = 480; mode[2] = 60; mode[3] = 0x16;
    return 0;
}
std::int32_t HP6_STDCALL fake_check_device_type(void*, std::uint32_t, std::uint32_t, std::uint32_t,
                                                std::uint32_t, std::uint32_t) {
    return 0;
}
int create_device_calls = 0;
std::int32_t HP6_STDCALL fake_create_device(void*, std::uint32_t, std::uint32_t, std::uint32_t,
                                            std::uint32_t, void* params, void* out_device) {
    (void)params;
    ++create_device_calls;
    *reinterpret_cast<void**>(out_device) = &fake_device;
    return 0;
}

std::uint32_t HP6_STDCALL fake_get_available_texture_mem(void*) { return 16u << 20; }
std::int32_t HP6_STDCALL fake_get_swap_chain(void*, std::uint32_t, void* out) {
    *reinterpret_cast<void**>(out) = &fake_swap_chain;
    return 0;
}
std::int32_t HP6_STDCALL fake_get_back_buffer(void*, std::uint32_t, std::uint32_t, void* out) {
    *reinterpret_cast<void**>(out) = &fake_surface;
    return 0;
}
std::int32_t HP6_STDCALL fake_get_desc(void*, void* out) {
    std::memset(out, 0, 0x20);
    auto* desc = reinterpret_cast<std::uint32_t*>(out);
    desc[0] = 0x15; desc[6] = 640; desc[7] = 480;
    return 0;
}
std::int32_t HP6_STDCALL fake_clear(void*, std::uint32_t, void*, std::uint32_t, std::uint32_t,
                                    float, std::uint32_t) {
    return 0;
}
std::int32_t HP6_STDCALL fake_present(void*, void*, void*, void*, void*) { return 0; }
std::int32_t HP6_STDCALL fake_create_depth_stencil(void*, std::uint32_t, std::uint32_t,
                                                   std::uint32_t, std::uint32_t, std::uint32_t,
                                                   std::uint32_t, void* out, std::uint32_t) {
    *reinterpret_cast<void**>(out) = &fake_depth_surface;
    return 0;
}
std::int32_t HP6_STDCALL fake_create_texture(void*, std::uint32_t, std::uint32_t, std::uint32_t,
                                             std::uint32_t, std::uint32_t, std::uint32_t, void* out,
                                             std::uint32_t) {
    *reinterpret_cast<void**>(out) = &fake_texture;
    return 0;
}
std::int32_t HP6_STDCALL fake_get_surface_level(void*, std::uint32_t, void* out) {
    *reinterpret_cast<void**>(out) = &fake_surface;
    return 0;
}
std::int32_t HP6_STDCALL fake_set_render_target(void*, std::uint32_t, void*) { return 0; }
std::int32_t HP6_STDCALL fake_set_depth_stencil(void*, void*) { return 0; }
std::int32_t HP6_STDCALL fake_set_render_state(void*, std::uint32_t, std::uint32_t) { return 0; }
std::int32_t HP6_STDCALL fake_set_sampler_state(void*, std::uint32_t, std::uint32_t, std::uint32_t) {
    return 0;
}
std::int32_t HP6_STDCALL fake_set_texture_stage_state(void*, std::uint32_t, std::uint32_t,
                                                       std::uint32_t) {
    return 0;
}
std::int32_t HP6_STDCALL fake_create_vertex_buffer(void*, std::uint32_t, std::uint32_t,
                                                   std::uint32_t, std::uint32_t, void* out,
                                                   std::uint32_t) {
    *reinterpret_cast<void**>(out) = &fake_vertex_buffer;
    return 0;
}
std::int32_t HP6_STDCALL fake_test_cooperative_level(void*) { return 0; } // S_OK
std::int32_t HP6_STDCALL fake_reset(void*, void*) { return 0; } // S_OK

extern "C" void* HP6_STDCALL fake_Direct3DCreate9(std::uint32_t) {
    return nullptr; // never actually reached: DAT_00cf194c is pre-set below
}

bool terminate_called = false;
void* terminate_process_arg = nullptr;
std::uint32_t terminate_exit_code = 0;
void recording_terminate(void* process, std::uint32_t exit_code) {
    terminate_called = true;
    terminate_process_arg = process;
    terminate_exit_code = exit_code;
}

} // namespace

#define HP6_X5(x) x, x, x, x, x
#define HP6_X10(x) HP6_X5(x), HP6_X5(x)

using hp6::presentation::CommandTableEntry;
const CommandTableEntry kUnusedCommand{"", &unused_command_handler};
const CommandTableEntry kTriggerBootupFlow{"TriggerBootupFlow",
                                           &hp6::presentation::handle_trigger_bootup_flow};
extern const CommandTableEntry* const PTR_PTR_00876ab0[93] = {
    &kTriggerBootupFlow,
    HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand),
    HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand),
    HP6_X10(&kUnusedCommand), HP6_X10(&kUnusedCommand),
    &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand,
    &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand, &kUnusedCommand,
    &kUnusedCommand, &kUnusedCommand,
};

using hp6::presentation::ScreenTaskEntry;
void* unused_screen_vtable[1] = {};
const ScreenTaskEntry kUnusedScreen{unused_screen_vtable, ""};
extern const ScreenTaskEntry* const PTR_PTR_00870948[26] = {
    HP6_X10(&kUnusedScreen), HP6_X10(&kUnusedScreen), HP6_X5(&kUnusedScreen), &kUnusedScreen,
};

extern "C" void* HP6_STDCALL Direct3DCreate9(std::uint32_t version) {
    return fake_Direct3DCreate9(version);
}

int main() {
    using namespace hp6::platform;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // ---- wire the fake D3D9/device/swap-chain/surface/texture chain -------
    d3d9_vtable[0x10 / 4] = reinterpret_cast<void*>(&fake_get_adapter_count);
    d3d9_vtable[0x14 / 4] = reinterpret_cast<void*>(&fake_get_adapter_identifier);
    d3d9_vtable[0x38 / 4] = reinterpret_cast<void*>(&fake_get_device_caps);
    d3d9_vtable[0x18 / 4] = reinterpret_cast<void*>(&fake_get_adapter_mode_count);
    d3d9_vtable[0x1c / 4] = reinterpret_cast<void*>(&fake_enum_adapter_modes);
    d3d9_vtable[0x20 / 4] = reinterpret_cast<void*>(&fake_get_adapter_display_mode);
    d3d9_vtable[0x24 / 4] = reinterpret_cast<void*>(&fake_check_device_type);
    d3d9_vtable[0x40 / 4] = reinterpret_cast<void*>(&fake_create_device);
    d3d9_vtable[8 / 4] = reinterpret_cast<void*>(&fake_release);
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
    device_vtable[0xc / 4] = reinterpret_cast<void*>(&fake_test_cooperative_level);
    device_vtable[0x40 / 4] = reinterpret_cast<void*>(&fake_reset);
    device_vtable[8 / 4] = reinterpret_cast<void*>(&fake_release);

    swap_chain_vtable[0x14 / 4] = reinterpret_cast<void*>(&fake_get_back_buffer);
    swap_chain_vtable[8 / 4] = reinterpret_cast<void*>(&fake_release);
    surface_vtable[0x30 / 4] = reinterpret_cast<void*>(&fake_get_desc);
    surface_vtable[8 / 4] = reinterpret_cast<void*>(&fake_release);
    depth_surface_vtable[0x30 / 4] = reinterpret_cast<void*>(&fake_get_desc);
    depth_surface_vtable[8 / 4] = reinterpret_cast<void*>(&fake_release);
    texture_vtable[0x48 / 4] = reinterpret_cast<void*>(&fake_get_surface_level);
    texture_vtable[8 / 4] = reinterpret_cast<void*>(&fake_release);
    vertex_buffer_vtable[8 / 4] = reinterpret_cast<void*>(&fake_release);

    PTR_FUN_0083e55c[4] = reinterpret_cast<void*>(&fake_create_heap);

    DAT_00fcadf0 = &fake_task_handler;
    PTR_LAB_00828360[0] = bootup_task_vtable[0];
    PTR_LAB_00828360[1] = bootup_task_vtable[1];

    // ---- avoid an infinite real message loop: a prior WM_QUIT makes
    // run_message_loop return 0 immediately, exactly like
    // tests/message_loop_smoke.cpp's own "guard" scenario -------------------
    DAT_00cef68d = 1;

    char command_line[] = "";
    const std::int32_t result = run_win_main(GetModuleHandleA(nullptr), command_line,
                                             &recording_terminate);

    expect("window_was_created", DAT_00cef694 != nullptr);
    expect("device_focus_window_matches_game_window",
           DAT_009978e4 == static_cast<std::uint32_t>(
               reinterpret_cast<std::uintptr_t>(DAT_00cef694)));
    expect("bootup_flow_dispatched", DAT_00cef68e == 1);
    // DAT_00cf1944 is checked via the creation counter, not its final
    // value: shutdown_render_device (part of run_win_main's own
    // success-path cleanup) releases and nulls it before this point.
    expect("device_was_created", create_device_calls >= 1);
    expect("gate_passed_and_terminate_invoked", terminate_called);
    expect("terminate_exit_code_is_success", terminate_exit_code == 1);
    expect("message_loop_result_is_guard_zero", result == 0);

    if (DAT_00cef694 != nullptr) {
        DestroyWindow(static_cast<HWND>(DAT_00cef694));
    }
    UnregisterClassA("HalfBloodPrinceMainWndClass", GetModuleHandleA(nullptr));

    // ---- parse_command_line_overrides (0x00ffa0d0) -------------------------
    // Independent of run_win_main above: a plain "-flag", a simple
    // "-key=value" pair, and a quoted value with an embedded space (the
    // quote-aware tokenizer, FUN_005e0630, must strip the quotes and not
    // stop at the space inside them).
    {
        DAT_00d644e8 = 0;
        DAT_00d644ec = 0;
        char overrides[] = "-windowed -lang=en -title=\"Half Blood Prince\"";
        hp6::platform::parse_command_line_overrides(overrides);

        expect("overrides_pair_count", DAT_00d644ec == 3);
        expect("overrides_plain_count", DAT_00d644e8 == 0);
        expect("overrides_windowed_key", std::strcmp(DAT_00d643e8[0], "windowed") == 0);
        expect("overrides_windowed_no_value", DAT_00d64468[0] == nullptr);
        expect("overrides_lang_key", std::strcmp(DAT_00d643e8[1], "lang") == 0);
        expect("overrides_lang_value", std::strcmp(DAT_00d64468[1], "en") == 0);
        expect("overrides_title_key", std::strcmp(DAT_00d643e8[2], "title") == 0);
        expect("overrides_title_quoted_value_with_space",
               std::strcmp(DAT_00d64468[2], "Half Blood Prince") == 0);
    }

    if (failures != 0) {
        std::printf("winmain_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("winmain_smoke: all checks passed\n");
    return 0;
}
