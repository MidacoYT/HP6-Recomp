#include "render/ScreenOverlayRender.h"
#include "render/DeviceLostHandler.h"
#include "render/DeviceReset.h"
#include "render/RenderStateCache.h"

#include <cstdint>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern void* DAT_00cf1944; // IDirect3DDevice9* (also in DeviceCreate.cpp etc.)
extern void* DAT_00c75a7c; // shared dynamic vertex buffer (also in DeviceReset.cpp etc.)
extern void* DAT_00cf1948; // IDirect3DSwapChain9* (also in RenderTargetSetup.cpp)
extern void* DAT_00cf1958; // render-target texture, or the 0xbacb0ffe disabled sentinel

extern void* DAT_00cf1930; // cached vertex declaration handle (not yet traced)
extern void* DAT_00cf192c; // cached vertex shader handle (not yet traced)
extern std::uint32_t DAT_00866358; // last-bound stream-source vertex buffer (bookkeeping only)
extern std::uint32_t DAT_00cf1908;  // last-bound stream stride (bookkeeping only)
extern std::uint32_t DAT_00866360;  // bookkeeping word set after a draw (not yet traced)

extern std::uint32_t DAT_00cf19e4; // render_fade_overlay_quad's one-shot countdown
extern std::uint32_t DAT_00cf19e0; // render_fade_overlay_quad's one-shot countdown
extern std::int32_t DAT_00cf1994;  // FilterFlip setting (also read by WinMain.cpp)
extern std::uint32_t DAT_008663b0; // active display width (also in DeviceCreate.cpp)
extern std::uint32_t DAT_008663b4; // active display height (also in DeviceCreate.cpp)
extern float DAT_0080a8b0;         // untraced opaque float, also used by the quad's UV/position math
extern float DAT_0080a8d0;         // untraced opaque float
extern float DAT_00831c2c;         // untraced opaque float (likely 0.0f, matching the quad's z plane)
extern float DAT_007db234;         // untraced opaque float (likely 1.0f, matching rhw and a UV extent)

namespace hp6::render {
namespace {

#ifdef _MSC_VER
using VbLockCall = std::int32_t(__stdcall*)(void*, std::uint32_t, std::uint32_t, void**, std::uint32_t);
using VbUnlockCall = std::int32_t(__stdcall*)(void*);
using SetStreamSourceCall = std::int32_t(__stdcall*)(void*, std::uint32_t, void*, std::uint32_t, std::uint32_t);
using DrawPrimitiveCall = std::int32_t(__stdcall*)(void*, std::uint32_t, std::uint32_t, std::uint32_t);
using EndSceneCall = std::int32_t(__stdcall*)(void*);
using PresentCall = std::int32_t(__stdcall*)(void*, void*, void*, void*, void*);
using GetBackBufferCall = std::int32_t(__stdcall*)(void*, std::uint32_t, std::uint32_t, void*);
using ReleaseCall = std::int32_t(__stdcall*)(void*);
using BeginSceneCall = std::int32_t(__stdcall*)(void*);
#else
using VbLockCall = std::int32_t(*)(void*, std::uint32_t, std::uint32_t, void**, std::uint32_t);
using VbUnlockCall = std::int32_t(*)(void*);
using SetStreamSourceCall = std::int32_t(*)(void*, std::uint32_t, void*, std::uint32_t, std::uint32_t);
using DrawPrimitiveCall = std::int32_t(*)(void*, std::uint32_t, std::uint32_t, std::uint32_t);
using EndSceneCall = std::int32_t(*)(void*);
using PresentCall = std::int32_t(*)(void*, void*, void*, void*, void*);
using GetBackBufferCall = std::int32_t(*)(void*, std::uint32_t, std::uint32_t, void*);
using ReleaseCall = std::int32_t(*)(void*);
using BeginSceneCall = std::int32_t(*)(void*);
#endif

void* vtable_of(void* object) {
    return *reinterpret_cast<void* const*>(object);
}

constexpr std::uint32_t kPrimitiveTypeTriangleStrip = 5; // D3DPT_TRIANGLESTRIP
constexpr std::uintptr_t kRenderTargetTextureDisabled = 0xbacb0ffeu;

} // namespace

void submit_screen_quad(const float* data, std::uint32_t flags, std::int32_t vertex_count) {
    if (vertex_count >= 0x400) {
        return;
    }

    auto lock = *reinterpret_cast<VbLockCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00c75a7c)) + 0x2c);
    void* locked = nullptr;
    if (lock(DAT_00c75a7c, 0, 0, &locked, 0x2000) < 0) { // D3DLOCK_DISCARD
        return;
    }

    auto* out = static_cast<std::uint32_t*>(locked);
    const auto* in = reinterpret_cast<const std::uint32_t*>(data);
    std::int32_t cursor = 0;
    for (std::int32_t i = 0; i < vertex_count; ++i) {
        if ((flags & 1u) != 0u) {
            out[0] = in[cursor + 0];
            out[1] = in[cursor + 1];
            out[2] = in[cursor + 2];
            out[3] = in[cursor + 3];
            cursor += 4;
        }
        if ((flags & 4u) == 0u) {
            out[4] = 0xffffffffu;
        } else {
            out[4] = in[cursor];
            cursor += 1;
        }
        if ((flags & 8u) == 0u) {
            out[5] = 0;
            out[6] = 0;
        } else {
            out[5] = in[cursor + 0];
            out[6] = in[cursor + 1];
            cursor += 2;
        }
        out += 7;
    }

    auto unlock = *reinterpret_cast<VbUnlockCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00c75a7c)) + 0x30);
    unlock(DAT_00c75a7c);

    auto set_stream_source = *reinterpret_cast<SetStreamSourceCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 400);
    set_stream_source(DAT_00cf1944, 0, DAT_00c75a7c, 0, 0x1c);
    DAT_00866358 = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(DAT_00c75a7c));
    DAT_00cf1908 = 0x1c;

    set_vertex_declaration_cached(DAT_00cf1930);
    set_vertex_shader_cached(DAT_00cf192c, true);

    auto draw_primitive = *reinterpret_cast<DrawPrimitiveCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x144);
    draw_primitive(DAT_00cf1944, kPrimitiveTypeTriangleStrip, 0,
                   static_cast<std::uint32_t>(vertex_count - 2));
    DAT_00866360 = 0xfffffff3u;
}

bool try_begin_debug_overlay_scene() {
    if ((handle_device_lost() & 0xffu) == 0u) {
        return false;
    }
    // Redundant with handle_device_lost's own internal call (it refreshes
    // the same cache on every path that reaches here); reproduced anyway
    // for fidelity to the exported x86, which makes the same two calls.
    cache_available_texture_memory();

    auto begin_scene = *reinterpret_cast<BeginSceneCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0xa4);
    const std::int32_t result = begin_scene(DAT_00cf1944);
    ++DAT_00cf19e4;
    return result == 0;
}

void render_fade_overlay_quad(std::uint32_t color) {
    if (DAT_00cf19e4 == 0) {
        return;
    }
    --DAT_00cf19e4;

    if (DAT_00cf1958 != nullptr &&
        DAT_00cf1958 != reinterpret_cast<void*>(kRenderTargetTextureDisabled)) {
        void* back_buffer = nullptr;
        auto get_back_buffer = *reinterpret_cast<GetBackBufferCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1948)) + 0x14);
        get_back_buffer(DAT_00cf1948, 0, 0, &back_buffer);
        set_render_targets_cached(back_buffer, nullptr);
        set_render_state_cached(7, 0);
        set_render_state_cached(0xe, 0);
        set_texture_cached(0, DAT_00cf1958);
        auto release = *reinterpret_cast<ReleaseCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(back_buffer)) + 8);
        release(back_buffer);

        float x_inset = DAT_0080a8b0 / static_cast<float>(DAT_008663b0);
        float y_inset = DAT_0080a8b0 / static_cast<float>(DAT_008663b4);
        if (DAT_00cf1994 != 0) {
            y_inset = DAT_00831c2c;
            x_inset = DAT_00831c2c;
        }
        const float left = DAT_0080a8d0 - x_inset;
        const float right = DAT_007db234 - x_inset;
        const float bottom = DAT_007db234 + y_inset;
        const float top = DAT_0080a8d0 + y_inset;

        // 4 vertices x (x, y, z, rhw, colour, u, v); colour is written as
        // the same `color` for all four, matching the exported x86's own
        // per-vertex colour writes, so submit_screen_quad's flags (0xd)
        // tell it to read colour from this data rather than default to
        // opaque white.
        auto as_u32 = [](float value) { return *reinterpret_cast<std::uint32_t*>(&value); };
        const std::uint32_t staged[4 * 7] = {
            as_u32(left),  as_u32(top),    as_u32(DAT_00831c2c), as_u32(DAT_007db234), color,
            as_u32(DAT_00831c2c), as_u32(DAT_007db234),
            as_u32(right), as_u32(top),    as_u32(DAT_00831c2c), as_u32(DAT_007db234), color,
            as_u32(DAT_007db234), as_u32(DAT_007db234),
            as_u32(left),  as_u32(bottom), as_u32(DAT_00831c2c), as_u32(DAT_007db234), color,
            as_u32(DAT_00831c2c), as_u32(DAT_00831c2c),
            as_u32(right), as_u32(bottom), as_u32(DAT_00831c2c), as_u32(DAT_007db234), color,
            as_u32(DAT_007db234), as_u32(DAT_00831c2c),
        };
        submit_screen_quad(reinterpret_cast<const float*>(staged), 0xd, 4);
    }

    auto end_scene = *reinterpret_cast<EndSceneCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0xa8);
    end_scene(DAT_00cf1944);
    auto present = *reinterpret_cast<PresentCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x44);
    present(DAT_00cf1944, nullptr, nullptr, nullptr, nullptr);
    DAT_00cf19e0 = 0;
}

} // namespace hp6::render
