// Local behavior checks for FUN_005d8fc0, the real Win32 message loop
// (called once from WinMain after ShowWindow/UpdateWindow — see
// architecture/startup_path.md). This test targets the loop's own
// control flow (the one-shot quit guard, WM_QUIT handling) with
// DAT_00cf1944 left null so hp6::render::handle_device_lost is a no-op
// and the "no pending message" frame-pump branch is never taken in
// this process's own message queue during the test — that branch is
// already covered independently by tests/frame_clock_smoke.cpp
// (hp6::runtime::run_frame_tick) and tests/device_lost_handler_smoke.cpp.
#include "platform/MessageLoop.h"

#include <cstdint>
#include <cstdio>

#ifdef _WIN32
#include <windows.h>
#endif

// ---- MessageLoop.cpp's own extern dependencies ----------------------------
std::uint8_t DAT_00cef68d = 0;
std::uint8_t DAT_00866e2a = 0;
std::uint8_t DAT_00cef68f = 0;
std::int32_t DAT_00cef55c = 0;
std::uint32_t DAT_00d64570 = 0;
std::int32_t DAT_00d64574 = 0;
float DAT_0080c680 = 1.0f;
float DAT_0083b230 = 1.0f;
float DAT_008003ac = 1.0f;
std::uint8_t DAT_00cef69e = 0;
std::int32_t DAT_00866e24 = 0;
std::int32_t DAT_00cef6a0 = 0;
std::uint8_t DAT_00cef69c = 1;
std::uint8_t DAT_00cef69d = 1;
void* DAT_00fcade0 = nullptr;

// ---- hp6::presentation::copy_gsc_message_handles's own extern
// dependencies (only reached if DAT_00cef55c counts down to zero, not
// exercised by this test) ---------------------------------------------------
std::int32_t DAT_00d63ec0 = 0, DAT_00d63e80 = 0;
std::int32_t DAT_00d63e10 = 0, DAT_00d63e00 = 0, DAT_00d63e08 = 0;
std::int32_t DAT_00d63e90 = 0, DAT_00d63e50 = 0, DAT_00d63ea8 = 0;

// ---- hp6::presentation::initialize_engine_core_systems's own extern
// dependencies (pulled in transitively via EngineBootstrap.obj, never
// actually exercised by this test's two scenarios) ---------------------
std::int32_t DAT_00d63eb0 = 0, DAT_00d63df8 = 0, DAT_00d63e30 = 0;
std::int32_t DAT_00d63ea0 = 0, DAT_00d63de4 = 0;
std::int32_t DAT_00d63eb8 = 0, DAT_00d63e68 = 0, DAT_00d63ed0 = 0, DAT_00d63e58 = 0;
std::int32_t DAT_00d63e88 = 0, DAT_00d63e38 = 0, DAT_00d63e28 = 0, DAT_00d63ec8 = 0;
std::int32_t DAT_00d63e40 = 0, DAT_00d63e18 = 0, DAT_00d63e98 = 0;
std::int32_t DAT_00d63e70 = 0, DAT_00d63ed8 = 0, DAT_00d63e60 = 0;
std::uint32_t DAT_00cef884 = 0, DAT_00cef880 = 0;
std::uint32_t DAT_00fc8478 = 0;
void* DAT_00cef730 = nullptr;
void* PTR_PTR_0087eadc = nullptr;
void* DAT_00d6af14 = nullptr;
std::uint8_t DAT_00cef718 = 0, DAT_00cef69f = 0;
float _DAT_008a4474 = 60.0f;

// ---- hp6::runtime::run_frame_tick's own extern dependencies (never
// actually reached by this test's two scenarios, but linked
// transitively) --------------------------------------------------------
void* DAT_0089eb0c = nullptr;
void* DAT_00fc7f90 = nullptr;
std::int32_t DAT_0089eb14 = 0;
std::int32_t DAT_00cef790 = 0;
std::uint32_t DAT_00d64578 = 0;
std::int32_t DAT_00d6457c = 0;
std::int32_t DAT_00d644f0 = 0;
std::int32_t DAT_00d644f4 = 0;
std::uint8_t DAT_00cef72c = 0;
float DAT_00ba9504 = 0.0f;
float DAT_008482ec = 65536.0f;

// ---- hp6::render::handle_device_lost's own extern dependencies (kept a
// no-op throughout this test by leaving DAT_00cf1944 null — the rest is
// still needed because the linker resolves every symbol the pulled-in
// object files reference, whether or not this test's own two scenarios
// ever reach them at runtime) -----------------------------------------
void* DAT_00cf1944 = nullptr;
std::uint8_t DAT_00cf19dc = 0;
std::uint8_t DAT_00cf18c7 = 0;
std::uint32_t DAT_00c75538[14] = {};
void* DAT_00c75a7c = nullptr;
void* DAT_00bb0a40 = nullptr;
void* DAT_00ba14f8 = nullptr;
void* DAT_00cf1948 = nullptr;
void* DAT_00cf1954 = nullptr;
void* DAT_00cf1958 = nullptr;
void* DAT_00cf1960 = nullptr;
void* DAT_00cf19d8 = nullptr; // shutdown_render_device's own untraced auxiliary object
void* DAT_00cf194c = nullptr; // IDirect3D9* (also in DeviceCreate.cpp)
std::uint32_t DAT_00cf19c8 = 0; // adapter count (also in DeviceCreate.cpp)
void* DAT_00cf195c = nullptr;
std::int32_t DAT_00cf1938 = 0;
std::uint8_t DAT_00cf19df = 0;
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
std::int32_t DAT_00cf1998 = 0;
std::uint8_t DAT_008663d4 = 0;
std::uint8_t DAT_00865287 = 0;
std::uint32_t DAT_008663b0 = 0;
std::uint32_t DAT_008663b4 = 0;
std::uint32_t DAT_00b170b8 = 0;

// ---- hp6::platform::service_holder dependencies (reached when
// DAT_00cef69e is set, not exercised by this test) --------------------------
void* PTR_FUN_0083e55c = nullptr;
void* PTR_FUN_0083e56c = nullptr;
void* DAT_00fcdd74 = nullptr;
void* DAT_00fcdd78 = nullptr;
void** DAT_00cef6bc = nullptr;
std::uint32_t DAT_00fd1254 = 0;
void cleanup_00711e60() {}
void* PTR_FUN_007159b0 = nullptr;
void* PTR_FUN_00715d0c = nullptr;
void* PTR_LAB_00715da8 = nullptr;
void* PTR_LAB_0071b520 = nullptr;
std::uint32_t DAT_008998c8 = 0;

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}
} // namespace

int main() {
    using namespace hp6::platform;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // ---- one-shot quit guard: a prior WM_QUIT makes this an immediate
    // no-op, clearing the flag -----------------------------------------------
    DAT_00cef68d = 1;
    const std::int32_t guard_result = run_message_loop();
    expect("guard_returns_zero", guard_result == 0);
    expect("guard_clears_flag", DAT_00cef68d == 0);

    // ---- WM_QUIT handling: posted before the loop runs, so
    // PeekMessageA finds it on the very first iteration, before the
    // frame-pump branch is ever reached -----------------------------------
#ifdef _WIN32
    PostQuitMessage(42);
    const std::int32_t quit_result = run_message_loop();
    expect("quit_returns_posted_wparam", quit_result == 42);
    expect("quit_clears_flag_too", DAT_00cef68d == 0);
#endif

    if (failures != 0) {
        std::printf("message_loop_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("message_loop_smoke: all checks passed\n");
    return 0;
}
