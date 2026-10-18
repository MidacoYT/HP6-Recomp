#include "render/DeviceCreate.h"
#include "render/DeviceReset.h"
#include "render/RenderHeap.h"
#include "platform/Diagnostics.h"
#include "runtime/FrameClock.h"

#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

#include <cstdint>
#include <cstdlib>
#include <cstring>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern void* DAT_00cf194c;            // IDirect3D9*
extern std::int32_t DAT_00cf19d0;     // adapter/mode table base (also in DeviceReset.cpp)
extern std::uint32_t DAT_00cf19c8;    // adapter count
extern std::int32_t DAT_008663d8;     // active display mode index (also in DeviceReset.cpp)
extern void* DAT_00cf1944;            // IDirect3DDevice9* (also in DeviceReset.cpp/RenderTargetSetup.cpp)
extern std::uint32_t DAT_00cf19d4;    // (also in DeviceReset.cpp)
extern std::uint8_t DAT_00865286;     // (also in DeviceReset.cpp/RenderTargetSetup.cpp)
extern void* DAT_00c75a7c;            // shared dynamic vertex buffer cache (also in RenderTargetSetup.cpp)
extern std::int32_t DAT_00cf19bc;     // escalation counter (also in DeviceReset.cpp)
extern std::uint32_t _DAT_00cf19c0;   // second capability byte, read from the same caps block
extern std::uint32_t DAT_00cf1964;    // cached available texture memory, ~MiB (also in DeviceReset.cpp)
extern std::uint8_t DAT_008663d5;     // quality-tier flag (also in DeviceReset.cpp)
extern std::uint32_t DAT_008663dc;    // texture/LOD scale (also in DeviceReset.cpp)
extern std::uint32_t DAT_00866364;    // multisample-quality setting

extern std::uint32_t DAT_009978e4;     // saved HWND (hFocusWindow for CreateDevice)
extern std::int32_t DAT_00cf1950;      // nonzero selects the overridden/custom resolution-list path (deferred)
extern std::uint32_t DAT_00cf19a8;     // active pixel-format selector (also in DisplayMode.cpp/RenderStateCache.cpp)
extern std::int32_t DAT_00cf198c;
extern std::int32_t DAT_00cf1990;
extern std::uint32_t DAT_00cf19cc;     // g_ResolutionCycleFoundIndex (independent IDA knowledge base name)
extern std::uint32_t DAT_00b170b8;     // depth-stencil format (also in RenderTargetSetup.cpp)
extern float DAT_008663ac;             // adapter display-mode aspect ratio
extern std::uint32_t _DAT_00b10894;    // copy of create_d3d9_device's param_5 (purpose not cross-referenced)
extern float _DAT_008a4474;
extern float DAT_0084859c;

extern std::uint32_t DAT_008663b0;     // active display width (also in RenderTargetSetup.cpp)
extern std::uint32_t DAT_008663b4;     // active display height (also in RenderTargetSetup.cpp)
extern std::int32_t DAT_00cf1998;      // render-to-texture disabled flag (also in RenderTargetSetup.cpp)
extern void* DAT_00bb0a40;             // scratch surface holder (also in RenderTargetSetup.cpp)
extern void* DAT_00ba14f8;             // cached depth-stencil surface (also in RenderTargetSetup.cpp)
extern void* DAT_00cf1948;             // IDirect3DSwapChain9* (also in RenderTargetSetup.cpp)
extern void* DAT_00cf1954;             // surface (also in RenderTargetSetup.cpp)
extern void* DAT_00cf195c;             // depth-stencil surface (also in RenderTargetSetup.cpp)
extern void* DAT_00cf1958;             // render-target texture, or sentinel (also in RenderTargetSetup.cpp)
extern void* DAT_00cf1960;             // depth-stencil-as-texture
extern std::uint8_t DAT_008663d4;      // feature flag (also in RenderTargetSetup.cpp)
extern std::uint8_t DAT_00865287;      // feature flag (also in RenderTargetSetup.cpp)

extern std::int32_t _DAT_00cf19ac;
extern std::int32_t _DAT_00cf19b0;
extern std::int32_t _DAT_00cf19b4;
extern std::int32_t _DAT_00cf19b8;
extern float DAT_00805f14;
extern float _DAT_00848588;

extern std::uint32_t DAT_00cf19e8;
extern std::uint32_t DAT_00cef794;
extern std::uint32_t DAT_00866a74;
extern std::uint32_t DAT_008663c4;
extern std::uint32_t DAT_00848598;
extern std::uint32_t DAT_00834bb0;
extern std::uint32_t DAT_00848594;
extern std::uint32_t _DAT_0082cf24;
extern std::uint32_t _DAT_0082d600;
extern std::uint32_t DAT_00848590;
extern std::uint32_t DAT_0084858c;
extern std::uint8_t DAT_00cf19dd;      // flag set when DAT_00cf1990 (vsync-ish) is non-zero

extern std::int32_t DAT_00cf1970;
extern std::int32_t DAT_00cf1978;
extern std::uint32_t DAT_008663e0;
extern std::int32_t DAT_00cf1980;
extern float _DAT_0083b230;
extern std::int32_t DAT_00cf1988;
extern std::int32_t DAT_00cf1984;
extern float _DAT_0082ab5c;
extern std::int32_t DAT_00cf197c;

extern std::uint32_t DAT_008663c8;     // copied brightness setting

extern std::uint8_t DAT_00d71250[0x268]; // fixed "RenderFrameworkMainHeap" object

// The real, persistent D3DPRESENT_PARAMETERS block (the "_Dst_00c75538"
// Ghidra names when it appears as a memset/write destination) — a real
// global, not a stack-local struct, since hp6::render::handle_device_lost
// (src/render/DeviceLostHandler.cpp) reuses these exact same 14 fields
// for its own Reset() call after a device-lost recovery.
extern std::uint32_t DAT_00c75538[14];
extern void* DAT_00d6af14;               // engine-service object; see header doc comment

// Default-value scratch blocks: Ghidra generates one symbol per distinct
// access point into what is, in the real binary, a single contiguous
// array/struct region. Modelled here as byte arrays with manual offset
// arithmetic to stay faithful to that single-block layout (same approach
// RenderStateCache.cpp documents for DAT_00881370).
extern std::uint8_t DAT_00b108a4[8 * 0x80]; // per-stage sampler-state defaults
extern std::uint8_t DAT_009968dc[8 * 0x80]; // per-stage texture-stage-state defaults
extern std::uint8_t DAT_00c751a4[0x320];    // render-state defaults block
extern std::uint32_t DAT_00bb09b4[4];       // scratch block zeroed by clear_scratch_light_block

namespace hp6::render {
namespace {

void* vtable_of(void* object) {
    return *reinterpret_cast<void* const*>(object);
}

struct DisplayModeEntry {
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t refresh_rate;
    std::uint32_t format;
};

// 0x006490b0 ("_PtFuncCompare", qsort comparator parameter). Sorts
// descending by the 4th stored word (format), then ascending by the 1st
// (width), then ascending by the 2nd (height).
int __cdecl display_mode_compare(const void* lhs, const void* rhs) {
    const auto* a = static_cast<const std::uint32_t*>(lhs);
    const auto* b = static_cast<const std::uint32_t*>(rhs);
    if (static_cast<std::int32_t>(b[3]) < static_cast<std::int32_t>(a[3])) return -1;
    if (static_cast<std::int32_t>(a[3]) < static_cast<std::int32_t>(b[3])) return 1;
    if (a[0] < b[0]) return -1;
    if (b[0] < a[0]) return 1;
    if (a[1] < b[1]) return -1;
    if (b[1] < a[1]) return 1;
    return 0;
}

#ifdef _MSC_VER
#define HP6_STDCALL __stdcall
#else
#define HP6_STDCALL
#endif

using GetAdapterCountCall = std::uint32_t(HP6_STDCALL*)(void*);
using GetAdapterIdentifierCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, std::uint32_t, void*);
using GetDeviceCapsCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, std::uint32_t, void*);
using GetAdapterModeCountCall = std::uint32_t(HP6_STDCALL*)(void*, std::uint32_t, std::uint32_t);
using EnumAdapterModesCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, std::uint32_t, std::uint32_t, void*);
using GetAdapterDisplayModeCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, void*);
using CheckDeviceTypeCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t);
using CreateDeviceCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, void*, void*);
using GetSwapChainCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, void*);
using GetBackBufferCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, std::uint32_t, void*);
using GetDescCall = std::int32_t(HP6_STDCALL*)(void*, void*);
using ClearCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, void*, std::uint32_t, std::uint32_t, float, std::uint32_t);
using PresentCall = std::int32_t(HP6_STDCALL*)(void*, void*, void*, void*, void*);
using CreateDepthStencilSurfaceCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, void*, std::uint32_t);
using CreateTextureCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, void*, std::uint32_t);
using ReleaseCall = void(HP6_STDCALL*)(void*);
using GetSurfaceLevelCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, void*);
using SetRenderTargetCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, void*);
using SetDepthStencilSurfaceCall = std::int32_t(HP6_STDCALL*)(void*, void*);
using SetSamplerStateCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, std::uint32_t, std::uint32_t);
using SetTextureStageStateCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, std::uint32_t, std::uint32_t);
using SetRenderStateCall = std::int32_t(HP6_STDCALL*)(void*, std::uint32_t, std::uint32_t);
using GetAvailableTextureMemCall = std::uint32_t(HP6_STDCALL*)(void*);

void set_render_state(void* device, std::uint32_t state, std::uint32_t value) {
    auto call = *reinterpret_cast<SetRenderStateCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(device)) + 0xe4);
    call(device, state, value);
}

void set_sampler_state(void* device, std::uint32_t stage, std::uint32_t type, std::uint32_t value) {
    auto call = *reinterpret_cast<SetSamplerStateCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(device)) + 0x114);
    call(device, stage, type, value);
}

void set_texture_stage_state(void* device, std::uint32_t stage, std::uint32_t type, std::uint32_t value) {
    auto call = *reinterpret_cast<SetTextureStageStateCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(device)) + 0x10c);
    call(device, stage, type, value);
}

} // namespace

extern "C" void* HP6_STDCALL Direct3DCreate9(std::uint32_t sdk_version);

std::int32_t enumerate_display_modes() {
    if (DAT_00cf19d0 != 0) {
        return 0;
    }
    if (DAT_00cf194c == nullptr) {
        DAT_00cf194c = Direct3DCreate9(0x20);
        if (DAT_00cf194c == nullptr) {
            return -0x7fffbffb; // 0x80004005, E_FAIL
        }
    }

    auto get_adapter_count = *reinterpret_cast<GetAdapterCountCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf194c)) + 0x10);
    DAT_00cf19c8 = get_adapter_count(DAT_00cf194c);

    void* table = std::malloc(static_cast<std::size_t>(DAT_00cf19c8) * 0x584);
    std::memset(table, 0, static_cast<std::size_t>(DAT_00cf19c8) * 0x584);
    DAT_00cf19d0 = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(table));

    auto get_identifier = *reinterpret_cast<GetAdapterIdentifierCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf194c)) + 0x14);
    auto get_caps = *reinterpret_cast<GetDeviceCapsCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf194c)) + 0x38);
    auto get_mode_count = *reinterpret_cast<GetAdapterModeCountCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf194c)) + 0x18);
    auto enum_modes = *reinterpret_cast<EnumAdapterModesCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf194c)) + 0x1c);

    for (std::uint32_t adapter = 0; adapter < DAT_00cf19c8; ++adapter) {
        auto* entry_base = static_cast<std::uint8_t*>(table) + adapter * 0x584;
        get_identifier(DAT_00cf194c, adapter, 0, entry_base);
        get_caps(DAT_00cf194c, adapter, 1, entry_base + 0x454); // D3DDEVTYPE_HAL

        if (*reinterpret_cast<std::int32_t*>(entry_base + 0x454) != 1) {
            continue; // D3DCAPS9.DeviceType != D3DDEVTYPE_HAL
        }

        const std::uint32_t formats[3] = {0x16, 0x17, 0x18}; // R5G6B5, X8R8G8B8, A8R8G8B8
        std::uint32_t total_modes = 0;
        for (std::uint32_t f = 0; f < 3; ++f) {
            total_modes += get_mode_count(DAT_00cf194c, adapter, formats[f]);
        }

        *reinterpret_cast<std::uint32_t*>(entry_base + 0x44c) = 0;
        auto* mode_array = static_cast<DisplayModeEntry*>(
            std::malloc(static_cast<std::size_t>(total_modes) * sizeof(DisplayModeEntry)));
        *reinterpret_cast<DisplayModeEntry**>(entry_base + 0x450) = mode_array;

        for (std::uint32_t f = 0; f < 3; ++f) {
            const std::uint32_t mode_count = get_mode_count(DAT_00cf194c, adapter, formats[f]);
            for (std::uint32_t m = 0; m < mode_count; ++m) {
                DisplayModeEntry mode{};
                enum_modes(DAT_00cf194c, adapter, formats[f], m, &mode);
                if (mode.width <= 0x13f || mode.height <= 199) {
                    continue;
                }
                auto& count = *reinterpret_cast<std::uint32_t*>(entry_base + 0x44c);
                std::uint32_t i = 0;
                while (i < count &&
                       (mode_array[i].width != mode.width || mode_array[i].height != mode.height ||
                        mode_array[i].refresh_rate != mode.refresh_rate)) {
                    ++i;
                }
                if (i == count) {
                    mode_array[i] = mode;
                    ++count;
                }
            }
        }

        std::qsort(mode_array, *reinterpret_cast<std::uint32_t*>(entry_base + 0x44c),
                   sizeof(DisplayModeEntry), display_mode_compare);
    }

    reset_hook_placeholder();
    return 0;
}

void copy_brightness_setting() {
    DAT_008663c8 = DAT_008663c4;
}

void update_quality_tier_flag() {
    if (DAT_00cf19bc < 3 || DAT_008663dc <= static_cast<std::uint32_t>(DAT_00cf1964 < 0x181)) {
        DAT_008663d5 = 1;
    } else {
        DAT_008663d5 = 0;
    }
}

void apply_multisample_quality(std::int32_t value) {
    if (value == 1) {
        DAT_00866364 = 0;
    } else if (value == 2) {
        DAT_00866364 = 2;
    }
}

void clear_scratch_light_block() {
    for (int i = 0; i < 4; ++i) {
        DAT_00bb09b4[i] = 0;
    }
}

void init_default_sampler_and_stage_state_table() {
    for (int stage = 0; stage < 8; ++stage) {
        auto* e = DAT_00b108a4 + stage * 0x80;
        *reinterpret_cast<std::uint32_t*>(e + 0x0) = 1;
        *reinterpret_cast<std::uint32_t*>(e + 0x4) = 1;
        *reinterpret_cast<std::uint32_t*>(e + 0x8) = 1;
        *reinterpret_cast<std::uint32_t*>(e + 0x10) = 1;
        *reinterpret_cast<std::uint32_t*>(e + 0x14) = 1;
        *reinterpret_cast<std::uint32_t*>(e + 0x18) = 0;
        *reinterpret_cast<std::uint32_t*>(e + 0x1c) = 0;
        *reinterpret_cast<std::uint32_t*>(e + 0x20) = 0;
        *reinterpret_cast<std::uint32_t*>(e + 0x24) = 1;
        *reinterpret_cast<std::uint32_t*>(e + 0xc) = 0;
    }
    for (int stage = 0; stage < 8; ++stage) {
        auto* e = DAT_009968dc + stage * 0x80;
        *reinterpret_cast<std::uint32_t*>(e + 0x0) = (stage == 0) ? 5u : 1u;
        *reinterpret_cast<std::uint32_t*>(e + 0x64) = 1;
        *reinterpret_cast<std::uint32_t*>(e + 0x4) = 2;
        *reinterpret_cast<std::uint32_t*>(e + 0x8) = 1;
        *reinterpret_cast<std::uint32_t*>(e + 0xc) = (stage == 0) ? 2u : 1u;
        *reinterpret_cast<std::uint32_t*>(e + 0x68) = 1;
        *reinterpret_cast<std::uint32_t*>(e + 0x10) = 0;
        *reinterpret_cast<std::uint32_t*>(e + 0x14) = 1;
        *reinterpret_cast<std::uint32_t*>(e + 0x6c) = 1;
        *reinterpret_cast<std::uint32_t*>(e + 0x5c) = 0;
        *reinterpret_cast<std::uint32_t*>(e + 0x18) = 0;
        *reinterpret_cast<std::uint32_t*>(e + 0x1c) = 0;
        *reinterpret_cast<std::uint32_t*>(e + 0x24) = 0;
        *reinterpret_cast<std::uint32_t*>(e + 0x20) = 0;
        *reinterpret_cast<std::uint32_t*>(e + 0x54) = 0;
        *reinterpret_cast<std::uint32_t*>(e + 0x58) = 0;
        *reinterpret_cast<std::uint32_t*>(e + 0x28) = static_cast<std::uint32_t>(stage);
    }
    apply_default_sampler_and_stage_states();
}

void apply_default_sampler_and_stage_states() {
    for (int stage = 0; stage < 8; ++stage) {
        auto* e = DAT_00b108a4 + stage * 0x80;
        set_sampler_state(DAT_00cf1944, stage, 1, *reinterpret_cast<std::uint32_t*>(e + 0x0));
        set_sampler_state(DAT_00cf1944, stage, 2, *reinterpret_cast<std::uint32_t*>(e + 0x4));
        set_sampler_state(DAT_00cf1944, stage, 3, *reinterpret_cast<std::uint32_t*>(e + 0x8));
        set_sampler_state(DAT_00cf1944, stage, 5, *reinterpret_cast<std::uint32_t*>(e + 0x10));
        set_sampler_state(DAT_00cf1944, stage, 6, *reinterpret_cast<std::uint32_t*>(e + 0x14));
        set_sampler_state(DAT_00cf1944, stage, 7, *reinterpret_cast<std::uint32_t*>(e + 0x18));
        set_sampler_state(DAT_00cf1944, stage, 8, *reinterpret_cast<std::uint32_t*>(e + 0x1c));
        set_sampler_state(DAT_00cf1944, stage, 9, *reinterpret_cast<std::uint32_t*>(e + 0x20));
        set_sampler_state(DAT_00cf1944, stage, 10, *reinterpret_cast<std::uint32_t*>(e + 0x24));
        set_sampler_state(DAT_00cf1944, stage, 4, *reinterpret_cast<std::uint32_t*>(e + 0xc));
    }
    for (int stage = 0; stage < 8; ++stage) {
        auto* e = DAT_009968dc + stage * 0x80;
        set_texture_stage_state(DAT_00cf1944, stage, 1, *reinterpret_cast<std::uint32_t*>(e + 0x0));
        set_texture_stage_state(DAT_00cf1944, stage, 0x1a, *reinterpret_cast<std::uint32_t*>(e + 0x64));
        set_texture_stage_state(DAT_00cf1944, stage, 2, *reinterpret_cast<std::uint32_t*>(e + 0x4));
        set_texture_stage_state(DAT_00cf1944, stage, 3, *reinterpret_cast<std::uint32_t*>(e + 0x8));
        set_texture_stage_state(DAT_00cf1944, stage, 4, *reinterpret_cast<std::uint32_t*>(e + 0xc));
        set_texture_stage_state(DAT_00cf1944, stage, 0x1b, *reinterpret_cast<std::uint32_t*>(e + 0x68));
        set_texture_stage_state(DAT_00cf1944, stage, 5, *reinterpret_cast<std::uint32_t*>(e + 0x10));
        set_texture_stage_state(DAT_00cf1944, stage, 6, *reinterpret_cast<std::uint32_t*>(e + 0x14));
        set_texture_stage_state(DAT_00cf1944, stage, 0x1c, *reinterpret_cast<std::uint32_t*>(e + 0x6c));
        set_texture_stage_state(DAT_00cf1944, stage, 0x18, *reinterpret_cast<std::uint32_t*>(e + 0x5c));
        set_texture_stage_state(DAT_00cf1944, stage, 7, *reinterpret_cast<std::uint32_t*>(e + 0x18));
        set_texture_stage_state(DAT_00cf1944, stage, 8, *reinterpret_cast<std::uint32_t*>(e + 0x1c));
        set_texture_stage_state(DAT_00cf1944, stage, 10, *reinterpret_cast<std::uint32_t*>(e + 0x24));
        set_texture_stage_state(DAT_00cf1944, stage, 9, *reinterpret_cast<std::uint32_t*>(e + 0x20));
        set_texture_stage_state(DAT_00cf1944, stage, 0x16, *reinterpret_cast<std::uint32_t*>(e + 0x54));
        set_texture_stage_state(DAT_00cf1944, stage, 0x17, *reinterpret_cast<std::uint32_t*>(e + 0x58));
        set_texture_stage_state(DAT_00cf1944, stage, 0xb, *reinterpret_cast<std::uint32_t*>(e + 0x28));
    }
}

void init_default_render_state_table() {
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x278) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x2a4) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x2c8) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x2ec) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x31c) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x0) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x4) = 3;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x8) = 2;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1c) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x20) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x24) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x30) = 2;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x34) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x3c) = 3;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x40) = 4;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x44) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x48) = 8;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x4c) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x50) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x54) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x58) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x6c) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x70) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x74) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x78) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x7c) = 0x3f800000;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xa4) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xb4) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xb8) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xbc) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xc0) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xc4) = 8;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xc8) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xcc) = 0xffffffff;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xd0) = 0xffffffff;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xd4) = 0xffffffff;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1e4) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1e8) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1ec) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1f0) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1f4) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1f8) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1fc) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x200) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x204) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x208) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x210) = 0x101010;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x214) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x218) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x21c) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x220) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x228) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x22c) = 2;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x230) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x234) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x240) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x244) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x24c) = 0x3f800000;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x250) = 0x3f800000;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x254) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x258) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x25c) = 0x3f800000;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x260) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x264) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x268) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x26c) = 0xffffffff;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x270) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x27c) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x280) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x284) = 0xf;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x28c) = 0;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x290) = 1;
    *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x29c) = 1;
    apply_default_render_states();
}

void apply_default_render_states() {
    set_render_state(DAT_00cf1944, 0x17, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x40));
    set_render_state(DAT_00cf1944, 0x19, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x48));
    set_render_state(DAT_00cf1944, 0x1b, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x50));
    set_render_state(DAT_00cf1944, 0xf, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x20));
    set_render_state(DAT_00cf1944, 0x18, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x44));
    set_render_state(DAT_00cf1944, 0x13, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x30));
    set_render_state(DAT_00cf1944, 0x14, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x34));
    set_render_state(DAT_00cf1944, 0xe, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1c));
    set_render_state(DAT_00cf1944, 0x1a, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x4c));
    set_render_state(DAT_00cf1944, 9, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x8));
    set_render_state(DAT_00cf1944, 0xa8, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x284));
    set_render_state(DAT_00cf1944, 0x36, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xbc));
    set_render_state(DAT_00cf1944, 0x37, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xc0));
    set_render_state(DAT_00cf1944, 0x38, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xc4));
    set_render_state(DAT_00cf1944, 0x39, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xc8));
    set_render_state(DAT_00cf1944, 0x3a, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xcc));
    set_render_state(DAT_00cf1944, 0x3b, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xd0));
    set_render_state(DAT_00cf1944, 0xab, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x290));
    set_render_state(DAT_00cf1944, 0x1c, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x54));
    set_render_state(DAT_00cf1944, 0x23, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x70));
    set_render_state(DAT_00cf1944, 0x24, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x74));
    set_render_state(DAT_00cf1944, 0x25, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x78));
    set_render_state(DAT_00cf1944, 0x26, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x7c));
    set_render_state(DAT_00cf1944, 0x30, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xa4));
    set_render_state(DAT_00cf1944, 0x80, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1e4));
    set_render_state(DAT_00cf1944, 0x81, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1e8));
    set_render_state(DAT_00cf1944, 0x82, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1ec));
    set_render_state(DAT_00cf1944, 0x83, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1f0));
    set_render_state(DAT_00cf1944, 0x84, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1f4));
    set_render_state(DAT_00cf1944, 0x85, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1f8));
    set_render_state(DAT_00cf1944, 0x86, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x1fc));
    set_render_state(DAT_00cf1944, 0x87, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x200));
    set_render_state(DAT_00cf1944, 0x89, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x208));
    set_render_state(DAT_00cf1944, 0x1d, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x58));
    set_render_state(DAT_00cf1944, 0x8e, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x21c));
    set_render_state(DAT_00cf1944, 0x8d, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x218));
    set_render_state(DAT_00cf1944, 0x92, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x22c));
    set_render_state(DAT_00cf1944, 0x91, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x228));
    set_render_state(DAT_00cf1944, 0x93, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x230));
    set_render_state(DAT_00cf1944, 0x94, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x234));
    set_render_state(DAT_00cf1944, 0x8b, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x210));
    set_render_state(DAT_00cf1944, 0x9a, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x24c));
    set_render_state(DAT_00cf1944, 0x9b, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x250));
    set_render_state(DAT_00cf1944, 0x9c, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x254));
    set_render_state(DAT_00cf1944, 0x9d, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x258));
    set_render_state(DAT_00cf1944, 0x9e, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x25c));
    set_render_state(DAT_00cf1944, 0x9f, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x260));
    set_render_state(DAT_00cf1944, 0xa0, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x264));
    set_render_state(DAT_00cf1944, 0xa6, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x27c));
    set_render_state(DAT_00cf1944, 0xa3, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x270));
    set_render_state(DAT_00cf1944, 0x97, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x240));
    set_render_state(DAT_00cf1944, 0x22, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x6c));
    set_render_state(DAT_00cf1944, 8, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x4));
    set_render_state(DAT_00cf1944, 0x8f, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x220));
    set_render_state(DAT_00cf1944, 7, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x0));
    set_render_state(DAT_00cf1944, 0x34, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xb4));
    set_render_state(DAT_00cf1944, 0x35, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xb8));
    set_render_state(DAT_00cf1944, 0x16, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x3c));
    set_render_state(DAT_00cf1944, 0x3c, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0xd4));
    set_render_state(DAT_00cf1944, 0xa1, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x268));
    set_render_state(DAT_00cf1944, 0xa2, *reinterpret_cast<std::uint32_t*>(DAT_00c751a4 + 0x26c));
}

void apply_secondary_render_state_defaults() {
    reset_hook_placeholder();
    clear_scratch_light_block();
    init_default_sampler_and_stage_state_table();
    init_default_render_state_table();
}

void build_resolution_option_list_hard_cap() {
    std::abort();
}

void fatal_display_error() {
    std::abort();
}

std::int32_t create_d3d9_device(std::uint32_t width, std::uint32_t height,
                                 std::uint32_t /*param_3*/, std::uint32_t /*param_4*/,
                                 std::uint32_t param_5) {
    const std::uint32_t saved_hwnd = DAT_009978e4;

    if (DAT_00cf194c == nullptr) {
        DAT_00cf194c = Direct3DCreate9(0x20);
        if (DAT_00cf194c == nullptr) {
            fatal_display_error();
            return -0x7fffbffb; // 0x80004005, E_FAIL
        }
    }

    std::uint32_t adapter_width = 0;
    std::uint32_t adapter_height = 0;
    {
        auto get_display_mode = *reinterpret_cast<GetAdapterDisplayModeCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf194c)) + 0x20);
        std::uint32_t mode[4] = {};
        const auto hr = get_display_mode(DAT_00cf194c, 0, mode);
        if (hr < 0) {
            fatal_display_error();
            return -0x7fffbffb;
        }
        adapter_width = mode[0];
        adapter_height = mode[1];
        DAT_008663ac = static_cast<float>(adapter_width) / static_cast<float>(adapter_height);

        enumerate_display_modes();
        _DAT_00b10894 = param_5;
        _DAT_008a4474 = DAT_0084859c;

        const std::uint32_t adapter_format = mode[3];

        bool need_adapter_search =
            (DAT_008663d8 == static_cast<std::int32_t>(0xffffffff)) ||
            (DAT_00cf19c8 < static_cast<std::uint32_t>(DAT_008663d8));
        if (!need_adapter_search && static_cast<std::uint32_t>(DAT_008663d8) < DAT_00cf19c8) {
            need_adapter_search =
                *reinterpret_cast<std::int32_t*>(static_cast<std::uintptr_t>(DAT_00cf19d0) +
                                                  0x454 + DAT_008663d8 * 0x584) != 1;
        }
        if (need_adapter_search) {
            std::uint32_t found = 0;
            bool ok = false;
            while (found < DAT_00cf19c8 && !ok) {
                if (*reinterpret_cast<std::int32_t*>(static_cast<std::uintptr_t>(DAT_00cf19d0) +
                                                      0x454 + found * 0x584) == 1) {
                    DAT_008663d8 = static_cast<std::int32_t>(found);
                    ok = true;
                }
                ++found;
            }
            if (!ok) {
                hp6::platform::format_diagnostic_message("Couldn't find a hardware D3D device\n");
            }
        }
        (void)adapter_format;
    }

    std::int32_t display_format = 0x16;      // D3DFMT_R5G6B5
    std::uint32_t depth_stencil_format = 0x4b;
    std::int32_t backbuffer_format = 0x15;   // D3DFMT_A8R8G8B8
    DAT_00b170b8 = depth_stencil_format;

    {
        auto caps = compute_dynamic_table_size();
        DAT_00cf19bc = (*reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(caps) + 0xcc) >> 8) & 0xff;
        caps = compute_dynamic_table_size();
        _DAT_00cf19c0 = *reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(caps) + 0xcc) & 0xff;
    }

    std::uint32_t behavior_flags = 0;
    if (DAT_00cf1950 == 0) {
        if (backbuffer_format == 0x14 || backbuffer_format == 0x15 || backbuffer_format == 0x16) {
            DAT_00cf19a8 = 0x20;
        } else {
            DAT_00cf19a8 = 0x10;
        }
        if (DAT_00cf19a8 == 0x10) {
            display_format = 0x17;
            DAT_00b170b8 = 0x50;
        }
        backbuffer_format = /* local_4c, the adapter's current Format */ backbuffer_format;
    } else {
        if (DAT_00cf19a8 == 0) {
            DAT_00cf19a8 = 0x20;
        }
        auto caps = compute_dynamic_table_size();
        if (*reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(caps) + 0x98) < 3) {
            apply_multisample_quality(1);
            DAT_00cf198c = 1;
            DAT_00cf1990 = 1;
            DAT_00cf19a8 = 0x10;
            behavior_flags = 0x80000000;
            caps = compute_dynamic_table_size();
            if ((*reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(caps) + 0x20) & 0x800) == 0) {
                DAT_008663c4 = _DAT_0082cf24;
            }
        }
        build_resolution_option_list_hard_cap();
        if (DAT_00cf19a8 == 0x10) {
            DAT_00b170b8 = 0x50;
            backbuffer_format = *reinterpret_cast<std::int32_t*>(
                *reinterpret_cast<std::int32_t*>(static_cast<std::uintptr_t>(DAT_00cf19d0) + 0x450 +
                                                  DAT_008663d8 * 0x584) +
                0xc + DAT_00cf19cc * 0x10);
            DAT_00cf198c = 1;
            DAT_00cf1990 = 1;
            display_format = backbuffer_format;
        } else {
            DAT_00b170b8 = 0x4b;
            backbuffer_format = *reinterpret_cast<std::int32_t*>(
                *reinterpret_cast<std::int32_t*>(static_cast<std::uintptr_t>(DAT_00cf19d0) + 0x450 +
                                                  DAT_008663d8 * 0x584) +
                0xc + DAT_00cf19cc * 0x10);
            if (backbuffer_format == 0x16) {
                display_format = 0x15;
            }
        }
    }

    std::memset(DAT_00c75538, 0, sizeof(DAT_00c75538));
    auto check_device_type = *reinterpret_cast<CheckDeviceTypeCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf194c)) + 0x24);
    auto create_device = *reinterpret_cast<CreateDeviceCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf194c)) + 0x40);

    std::int32_t hr = check_device_type(DAT_00cf194c, DAT_008663d8, 1, backbuffer_format,
                                        display_format, 0);
    if (hr < 0) {
        if (DAT_00cf19a8 == 0x20) {
            DAT_00cf19a8 = 0x10;
            display_format = 0x17;
            DAT_00b170b8 = 0x50;
            backbuffer_format = 0x17;
            hr = check_device_type(DAT_00cf194c, DAT_008663d8, 1, 0x17, 0x17, 0);
        } else if (DAT_00cf19a8 == 0x10) {
            DAT_00cf19a8 = 0x20;
            display_format = 0x15;
            DAT_00b170b8 = 0x4b;
            backbuffer_format = 0x16;
            hr = check_device_type(DAT_00cf194c, DAT_008663d8, 1, 0x16, 0x15, 0);
        }
        if (hr < 0) {
            hp6::platform::format_diagnostic_message("Couldn't find a compatible display mode\n");
        }
    }

    std::uint32_t behavior_override = 0;
    if (DAT_00cf1950 == 0) {
        DAT_008663b0 = width;
        DAT_008663b4 = height;
        DAT_00c75538[0] = width;    // BackBufferWidth
        DAT_00c75538[1] = height;   // BackBufferHeight
        DAT_00c75538[7] = saved_hwnd; // hDeviceWindow
        behavior_override = 0x80000000;
    } else {
        const auto mode_entry = *reinterpret_cast<std::int32_t*>(
            static_cast<std::uintptr_t>(DAT_00cf19d0) + 0x450 + DAT_008663d8 * 0x584);
        DAT_00c75538[0] = *reinterpret_cast<std::uint32_t*>(
            static_cast<std::uintptr_t>(mode_entry) + DAT_00cf19cc * 0x10);
        DAT_00c75538[1] = *reinterpret_cast<std::uint32_t*>(
            static_cast<std::uintptr_t>(mode_entry) + 4 + DAT_00cf19cc * 0x10);
        DAT_00c75538[7] = saved_hwnd;
        behavior_override = behavior_flags;
    }
    DAT_00c75538[2] = static_cast<std::uint32_t>(backbuffer_format); // BackBufferFormat
    DAT_00c75538[3] = 1;                                             // BackBufferCount
    DAT_00c75538[4] = 0;                                             // MultiSampleType
    DAT_00c75538[5] = 0;                                             // MultiSampleQuality
    DAT_00c75538[6] = 1;                                             // SwapEffect (DISCARD)
    DAT_00c75538[8] = static_cast<std::uint32_t>(DAT_00cf1950 == 0); // Windowed
    DAT_00c75538[9] = 0;                                             // EnableAutoDepthStencil
    DAT_00c75538[10] = DAT_00b170b8;                                 // AutoDepthStencilFormat (unused, see below)
    DAT_00c75538[11] = 0;                                            // Flags
    DAT_00c75538[13] = behavior_override;                            // PresentationInterval

    std::uint32_t capability_flags =
        *reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(compute_dynamic_table_size()) + 0xc4);
    const std::uint8_t cap_hi = static_cast<std::uint8_t>(capability_flags >> 8);
    const char cap_lo = static_cast<char>(capability_flags);
    bool use_extra_behavior;
    if (((cap_hi == 1) && (cap_lo != 0)) || (cap_hi > 1)) {
        const std::uint32_t cap_bits =
            *reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(compute_dynamic_table_size()) + 0x1c);
        use_extra_behavior = (cap_bits & 0x10000) != 0;
    } else {
        use_extra_behavior = false;
    }
    if (use_extra_behavior) {
        DAT_00865286 = DAT_00865286 & 1;
    } else {
        DAT_00865286 = 0;
        DAT_00cf198c = 1;
    }

    const std::uint32_t create_flags = (use_extra_behavior ? 0x30u : 0u) + 0x20u;
    hr = create_device(DAT_00cf194c, DAT_008663d8, 1, saved_hwnd, create_flags,
                        DAT_00c75538, &DAT_00cf1944);
    if (hr < 0) {
        hr = create_device(DAT_00cf194c, DAT_008663d8, 1, saved_hwnd, 0x20u, DAT_00c75538,
                            &DAT_00cf1944);
        if (hr < 0) {
            fatal_display_error();
            return -0x7fffbffb;
        }
        DAT_00865286 = 0;
    }

    _DAT_008a4474 = static_cast<float>(*reinterpret_cast<std::uint32_t*>(
        static_cast<std::uintptr_t>(
            *reinterpret_cast<std::int32_t*>(static_cast<std::uintptr_t>(DAT_00cf19d0) + 0x450 +
                                              DAT_008663d8 * 0x584)) +
        8 + DAT_00cf19cc * 0x10));

    {
        auto get_texmem = *reinterpret_cast<GetAvailableTextureMemCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x10);
        const std::uint32_t bytes = get_texmem(DAT_00cf1944);
        DAT_00cf1964 = bytes >> 0x14;
    }
    update_quality_tier_flag();
    adjust_texture_quality_for_memory();

    if (DAT_00865286 == 0) {
        DAT_00cf19d4 = 0x10;
        DAT_00cf19e8 = DAT_00848598;
        DAT_00cef794 = DAT_00834bb0;
        DAT_00866a74 = DAT_00848594;
        DAT_008663c4 = _DAT_0082cf24;
        auto caps = compute_dynamic_table_size();
        if (*reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(caps) + 0x98) < 3) {
            caps = compute_dynamic_table_size();
            if ((*reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(caps) + 0x20) & 0x800) == 0) {
                DAT_008663c4 = _DAT_0082d600;
                DAT_00cf19e8 = DAT_00848590;
                DAT_00cef794 = DAT_00834bb0;
                DAT_00866a74 = DAT_0084858c;
            }
            DAT_008663dc = 0x40;
        }
    } else {
        DAT_00cf19d4 = 0;
    }

    if (DAT_00cf1970 != 0) {
        if (DAT_00cf19a8 == 0x10 && DAT_00cf1970 > 2) {
            DAT_00cf1970 = 2;
        }
        apply_multisample_quality(DAT_00cf1970);
    }
    if (DAT_00cf1978 != 0) {
        DAT_008663e0 = static_cast<std::uint32_t>(DAT_00cf1978);
    }
    if (DAT_00cf1980 != 0) {
        DAT_00cf19e8 = static_cast<std::uint32_t>(static_cast<float>(DAT_00cf1980) * _DAT_0083b230);
    }
    if (DAT_00cf1988 != 0) {
        DAT_00866a74 = static_cast<std::uint32_t>(DAT_00cf1988);
    }
    if (DAT_00cf1984 != 0) {
        DAT_00cef794 = static_cast<std::uint32_t>(static_cast<float>(DAT_00cf1984) * _DAT_0082ab5c);
    }
    if (DAT_00cf197c != 0) {
        DAT_008663c4 = static_cast<std::uint32_t>(DAT_00cf197c);
    }

    copy_brightness_setting();
    if (DAT_00cf1990 != 0) {
        DAT_00cf19dd = 1;
    }
    apply_secondary_render_state_defaults();

    auto get_swap_chain = *reinterpret_cast<GetSwapChainCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x38);
    get_swap_chain(DAT_00cf1944, 0, &DAT_00cf1948);
    auto get_back_buffer = *reinterpret_cast<GetBackBufferCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1948)) + 0x14);
    get_back_buffer(DAT_00cf1948, 0, 0, &DAT_00cf1954);
    DAT_00bb0a40 = DAT_00cf1954;

    std::uint32_t back_buffer_desc[8] = {};
    auto get_desc = *reinterpret_cast<GetDescCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00bb0a40)) + 0x30);
    get_desc(DAT_00bb0a40, back_buffer_desc);
    DAT_008663b0 = back_buffer_desc[6];
    DAT_008663b4 = back_buffer_desc[7];
    _DAT_00cf19ac = static_cast<std::int32_t>(static_cast<float>(DAT_008663b0) * DAT_00805f14);
    _DAT_00cf19b0 = static_cast<std::int32_t>(static_cast<float>(DAT_008663b4) * DAT_00805f14);
    _DAT_00cf19b4 = static_cast<std::int32_t>(static_cast<float>(DAT_008663b0) * _DAT_00848588);
    _DAT_00cf19b8 = static_cast<std::int32_t>(static_cast<float>(DAT_008663b4) * _DAT_00848588);

    auto clear = *reinterpret_cast<ClearCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0xac);
    auto present = *reinterpret_cast<PresentCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x44);
    clear(DAT_00cf1944, 0, nullptr, 1, 0, 1.0f, 0);
    present(DAT_00cf1944, nullptr, nullptr, nullptr, nullptr);
    clear(DAT_00cf1944, 0, nullptr, 1, 0, 1.0f, 0);
    present(DAT_00cf1944, nullptr, nullptr, nullptr, nullptr);

    auto create_depth_stencil = *reinterpret_cast<CreateDepthStencilSurfaceCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x74);
    create_depth_stencil(DAT_00cf1944, DAT_008663b0, DAT_008663b4, DAT_00b170b8, 0, 0, 0,
                          &DAT_00cf195c, 0);

    std::uint32_t depth_desc[8] = {};
    auto get_desc2 = *reinterpret_cast<GetDescCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf195c)) + 0x30);
    get_desc2(DAT_00cf195c, depth_desc);

    if (DAT_00cf1998 != 0) {
        DAT_00cf1958 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xbacb0ffeu));
    } else if (static_cast<bool>(DAT_008663d4) && static_cast<bool>(DAT_00865287) &&
               ((*reinterpret_cast<std::uint32_t*>(
                     static_cast<std::uintptr_t>(compute_dynamic_table_size()) + 0x3c) &
                 0x100) != 0 ||
                (*reinterpret_cast<std::uint32_t*>(
                     static_cast<std::uintptr_t>(compute_dynamic_table_size()) + 0x3c) &
                 2) == 0)) {
        auto create_texture = *reinterpret_cast<CreateTextureCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x5c);
        void* render_target_texture = nullptr;
        auto hr2 = create_texture(DAT_00cf1944, DAT_008663b0, DAT_008663b4, 1, 1,
                                   back_buffer_desc[0], 0, &render_target_texture, 0);
        if (hr2 >= 0) {
            auto release = *reinterpret_cast<ReleaseCall const*>(
                static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1954)) + 8);
            release(DAT_00cf1954);
            auto get_level = *reinterpret_cast<GetSurfaceLevelCall const*>(
                static_cast<const std::uint8_t*>(vtable_of(render_target_texture)) + 0x48);
            get_level(render_target_texture, 0, &DAT_00bb0a40);
            DAT_00cf1958 = render_target_texture;
            DAT_00cf1954 = DAT_00bb0a40;
        }
        void* depth_texture = nullptr;
        auto hr3 = create_texture(DAT_00cf1944, DAT_008663b0, DAT_008663b4, 1, 2,
                                   depth_desc[0], 0, &depth_texture, 0);
        if (hr3 >= 0) {
            auto release2 = *reinterpret_cast<ReleaseCall const*>(
                static_cast<const std::uint8_t*>(vtable_of(DAT_00cf195c)) + 8);
            release2(DAT_00cf195c);
            auto get_level2 = *reinterpret_cast<GetSurfaceLevelCall const*>(
                static_cast<const std::uint8_t*>(vtable_of(depth_texture)) + 0x48);
            get_level2(depth_texture, 0, &DAT_00ba14f8);
            DAT_00cf1960 = depth_texture;
            DAT_00cf195c = DAT_00ba14f8;
        }
    } else {
        DAT_00cf1958 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xbacb0ffeu));
    }

    DAT_00ba14f8 = DAT_00cf195c;
    auto set_render_target = *reinterpret_cast<SetRenderTargetCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x94);
    set_render_target(DAT_00cf1944, 0, DAT_00bb0a40);
    auto set_depth_stencil = *reinterpret_cast<SetDepthStencilSurfaceCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x9c);
    set_depth_stencil(DAT_00cf1944, DAT_00ba14f8);

    clear(DAT_00cf1944, 0, nullptr, 3, 0, 1.0f, 0);
    present(DAT_00cf1944, nullptr, nullptr, nullptr, nullptr);

    create_dynamic_vertex_buffer();
    return 0;
}

namespace {

// 0x00d6af14's single write site is outside any function Ghidra could
// name (see DeviceCreate.h's doc comment on
// create_and_configure_render_device); resolving it requires
// reconstructing FUN_005dcdf0 first. Unlike the two D3D9 failure-path
// stubs further up this file, this call sits on
// create_and_configure_render_device's unconditional success path —
// aborting here would make every caller untestable — so this is a
// documented no-op rather than a fabrication of the real object's
// behavior.
void invoke_engine_service_hook(void*) {}

} // namespace

std::int32_t create_and_configure_render_device(std::uint32_t width, std::uint32_t height) {
    hp6::runtime::reset_frame_pump_state();
    initialize_render_framework_heap(DAT_00d71250);
    const std::int32_t result = create_d3d9_device(width, height, 0, 0, 6);
    if (result == -0x7fffbffb) { // 0x80004005, E_FAIL
#ifdef _WIN32
        ShowCursor(TRUE);
#endif
    }
    invoke_engine_service_hook(DAT_00d6af14);
    return 0;
}

void run_post_message_loop_cleanup_primary(std::int32_t /*param*/) {
    invoke_engine_service_hook(DAT_00d6af14); // real target: vtable slot +0xc
}

void run_post_message_loop_cleanup_secondary() {
    invoke_engine_service_hook(DAT_00d6af14); // real target: vtable slot +4
}

} // namespace hp6::render
