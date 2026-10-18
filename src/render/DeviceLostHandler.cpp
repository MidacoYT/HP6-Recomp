#include "render/DeviceLostHandler.h"
#include "render/DeviceReset.h"
#include "render/RenderTargetSetup.h"
#include "platform/Diagnostics.h"

#include <cstdint>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern void* DAT_00cf1944;   // IDirect3DDevice9* (also in DeviceReset.cpp etc.)
extern std::uint8_t DAT_00cf19dc; // forced-reset flag
extern std::uint8_t DAT_00cf18c7; // "a reset is in progress" flag
extern std::uint32_t DAT_00c75538[14]; // the real D3DPRESENT_PARAMETERS block

extern void* DAT_00c75a7c; // shared dynamic vertex buffer (also in DeviceReset.cpp)
extern void* DAT_00bb0a40; // scratch surface holder (also in RenderTargetSetup.cpp)
extern void* DAT_00ba14f8; // cached depth-stencil surface (also in RenderTargetSetup.cpp)
extern void* DAT_00cf1948; // IDirect3DSwapChain9* (also in RenderTargetSetup.cpp)
extern void* DAT_00cf1954; // surface (also in RenderTargetSetup.cpp)
extern void* DAT_00cf1958; // render-target texture, or sentinel (also in RenderTargetSetup.cpp)
extern void* DAT_00cf1960; // depth-stencil-as-texture
extern void* DAT_00cf195c; // depth-stencil surface (also in RenderTargetSetup.cpp)

extern std::int32_t DAT_00cf1938; // deferred: thunk_FUN_01002630's own guard
extern std::uint8_t DAT_00cf19df; // deferred: thunk_FUN_01003a40's own guard

extern void* DAT_00cf19d8;         // untraced auxiliary object, shutdown_render_device only
extern void* DAT_00cf194c;         // IDirect3D9* (also in DeviceCreate.cpp)
extern std::uint32_t DAT_00cf19c8; // adapter count (also in DeviceCreate.cpp)
extern std::int32_t DAT_00cf19d0;  // adapter/mode table base, as an int (also in DeviceCreate.cpp)

namespace hp6::render {
namespace {

#ifdef _MSC_VER
using TestCooperativeLevelCall = std::int32_t(__stdcall*)(void*);
using ResetCall = std::int32_t(__stdcall*)(void*, void*);
using GetBackBufferCall = std::int32_t(__stdcall*)(void*, std::uint32_t, std::uint32_t, void*);
using ReleaseCall = std::int32_t(__stdcall*)(void*);
using SetRenderTargetCall = std::int32_t(__stdcall*)(void*, std::uint32_t, void*);
using SetDepthStencilSurfaceCall = std::int32_t(__stdcall*)(void*, void*);
#else
using TestCooperativeLevelCall = std::int32_t(*)(void*);
using ResetCall = std::int32_t(*)(void*, void*);
using GetBackBufferCall = std::int32_t(*)(void*, std::uint32_t, std::uint32_t, void*);
using ReleaseCall = std::int32_t(*)(void*);
using SetRenderTargetCall = std::int32_t(*)(void*, std::uint32_t, void*);
using SetDepthStencilSurfaceCall = std::int32_t(*)(void*, void*);
#endif

void* vtable_of(void* object) {
    return *reinterpret_cast<void* const*>(object);
}

constexpr std::int32_t kDeviceLost = -0x7789f798;       // D3DERR_DEVICELOST
constexpr std::int32_t kDeviceNotReset = -0x7789f797;   // D3DERR_DEVICENOTRESET
constexpr std::int32_t kDriverInternalError = -0x7789f7d9;
constexpr std::int32_t kOutOfMemory = -0x7ff8fff2;
constexpr std::int32_t kOutOfVideoMemory = -0x7789fe84;
constexpr std::int32_t kInvalidCall = -0x7789f794;

void release_if_present(void*& object) {
    if (object != nullptr) {
        auto release = *reinterpret_cast<ReleaseCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(object)) + 8);
        release(object);
        object = nullptr;
    }
}

} // namespace

std::int32_t set_render_targets_cached(void* render_target, void* depth_stencil) {
    if (render_target == DAT_00bb0a40 && depth_stencil == DAT_00ba14f8) {
        return 0;
    }
    auto set_rt = *reinterpret_cast<SetRenderTargetCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x94);
    const std::int32_t hr = set_rt(DAT_00cf1944, 0, render_target);
    auto set_ds = *reinterpret_cast<SetDepthStencilSurfaceCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x9c);
    set_ds(DAT_00cf1944, depth_stencil);
    DAT_00bb0a40 = render_target;
    DAT_00ba14f8 = depth_stencil;
    return hr;
}

void release_dynamic_render_resources() {
    release_if_present(DAT_00c75a7c);

    // NOT YET INTEGRATED: thunk_FUN_01002630 (guarded by DAT_00cf1938),
    // a documented deferred no-op.

    DAT_00bb0a40 = nullptr;
    DAT_00ba14f8 = nullptr;

    if (DAT_00cf1958 != nullptr &&
        DAT_00cf1958 != reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xbacb0ffeu))) {
        void* back_buffer = nullptr;
        auto get_back_buffer = *reinterpret_cast<GetBackBufferCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1948)) + 0x14);
        get_back_buffer(DAT_00cf1948, 0, 0, &back_buffer);
        release_if_present(back_buffer);
        release_if_present(DAT_00cf1958);
    }
    release_if_present(DAT_00cf1960);
    release_if_present(DAT_00cf1954);
    release_if_present(DAT_00cf195c);

    // NOT YET INTEGRATED: thunk_FUN_01003a40 (guarded by DAT_00cf19df),
    // a documented deferred no-op.
}

std::uint32_t handle_device_lost() {
    if (DAT_00cf1944 == nullptr) {
        return 1;
    }

    cache_available_texture_memory();

    auto test_level = *reinterpret_cast<TestCooperativeLevelCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0xc);
    const std::int32_t level = test_level(DAT_00cf1944);

    if (level >= 0 && DAT_00cf19dc == 0) {
        prepare_render_target_texture();
        DAT_00cf19dc = 0;
        DAT_00cf18c7 = 0;
        return 1;
    }

    DAT_00cf18c7 = 1;
    if (level == kDeviceLost) {
        Sleep(0x32);
        return 0;
    }
    if (level != kDeviceNotReset && DAT_00cf19dc == 0) {
        hp6::platform::format_diagnostic_message("Invalid device lost state %d\n");
    }

    release_dynamic_render_resources();

    auto reset = *reinterpret_cast<ResetCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x40);
    std::int32_t reset_hr = reset(DAT_00cf1944, DAT_00c75538);
    if (reset_hr < 0) {
        if (reset_hr == kDriverInternalError) {
            hp6::platform::format_diagnostic_message("Reset: Driver internal error\n");
        } else if (reset_hr == kOutOfMemory) {
            hp6::platform::format_diagnostic_message("Reset: Out of memory\n");
        } else if (reset_hr == kOutOfVideoMemory) {
            hp6::platform::format_diagnostic_message("Reset: Out of video memory\n");
        } else if (reset_hr == kDeviceLost) {
            Sleep(0x32);
        } else if (reset_hr == kInvalidCall) {
            hp6::platform::format_diagnostic_message("Reset: Invalid call\n");
        }
        return static_cast<std::uint32_t>(reset_hr) & 0xffffff00u;
    }

    prepare_render_target_texture();
    DAT_00cf19dc = 0;
    DAT_00cf18c7 = 0;
    return 1;
}

void shutdown_render_device() {
    release_if_present(DAT_00cf19d8);
    release_if_present(DAT_00c75a7c);

    for (std::uint32_t i = 0; i < DAT_00cf19c8; ++i) {
        std::free(*reinterpret_cast<void**>(
            static_cast<std::uintptr_t>(DAT_00cf19d0) + 0x450 + i * 0x584));
    }

    DAT_00bb0a40 = nullptr;
    DAT_00ba14f8 = nullptr;

    if (DAT_00cf1958 != nullptr &&
        DAT_00cf1958 != reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xbacb0ffeu))) {
        void* back_buffer = nullptr;
        auto get_back_buffer = *reinterpret_cast<GetBackBufferCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1948)) + 0x14);
        get_back_buffer(DAT_00cf1948, 0, 0, &back_buffer);
        set_render_targets_cached(back_buffer, nullptr);
        release_if_present(back_buffer);
        release_if_present(DAT_00cf1958);
    }
    release_if_present(DAT_00cf1954);
    release_if_present(DAT_00cf1960);
    release_if_present(DAT_00cf195c);

    std::free(reinterpret_cast<void*>(static_cast<std::uintptr_t>(DAT_00cf19d0)));
    DAT_00cf19d0 = 0;

    if (DAT_00cf1944 != nullptr) {
        auto release = *reinterpret_cast<ReleaseCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 8);
        const std::int32_t refs = release(DAT_00cf1944);
        if (refs != 0) {
            hp6::platform::format_diagnostic_message("Device refs %d\n", refs);
        }
        DAT_00cf1944 = nullptr;
    }

    if (DAT_00cf194c != nullptr) {
        auto release = *reinterpret_cast<ReleaseCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf194c)) + 8);
        const std::int32_t refs = release(DAT_00cf194c);
        if (refs != 0) {
            hp6::platform::format_diagnostic_message("D3D refs %d\n", refs);
        }
        DAT_00cf194c = nullptr;
    }
}

} // namespace hp6::render
