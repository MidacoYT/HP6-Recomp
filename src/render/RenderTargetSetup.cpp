#include "render/RenderTargetSetup.h"
#include "render/DeviceReset.h"
#include "render/RenderStateCache.h"

#include <cstddef>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern void* DAT_00cf1944;   // IDirect3DDevice9*
extern void* DAT_00c75a7c;   // shared dynamic vertex buffer cache
extern void* DAT_00cf1948;   // IDirect3DSwapChain9*
extern void* DAT_00cf1954;   // surface: the back buffer, then the render-target texture's level 0
extern void* DAT_00cf1958;   // the render-target texture, or the sentinel 0xbacb0ffe when disabled
extern void* DAT_00cf195c;   // depth-stencil surface
extern void* DAT_00bb0a40;   // scratch surface holder
extern void* DAT_00ba14f8;   // cached depth-stencil surface
extern std::int32_t DAT_00cf1998;  // render-to-texture disabled flag
extern std::uint8_t DAT_008663d4;  // feature flag
extern std::uint8_t DAT_00865287;  // feature flag
extern std::uint32_t DAT_008663b0; // active display width
extern std::uint32_t DAT_008663b4; // active display height
extern std::uint32_t DAT_00b170b8; // depth-stencil format

namespace hp6::render {
namespace {

void* vtable_of(void* object) {
    return *reinterpret_cast<void* const*>(object);
}

} // namespace

// 0x0064a810. Every virtual call below was cross-checked against the
// real IDirect3DSwapChain9/IDirect3DSurface9/IDirect3DTexture9/
// IDirect3DDevice9 vtable layouts (Wine's d3d9.h, which follows the
// standard COM declaration order): GetBackBuffer (swap chain, index 5,
// +0x14), GetDesc (surface, index 12, +0x30), CreateDepthStencilSurface
// (device, index 29, +0x74), CreateTexture (device, index 23, +0x5c),
// Release (any COM object, index 2, +8), GetSurfaceLevel (texture,
// index 18, +0x48), SetRenderTarget (device, index 37, +0x94),
// SetDepthStencilSurface (device, index 39, +0x9c) — every offset's
// argument count and shape matches its real signature exactly.
//
// Sequence: recreate dynamic resources, lazily create the shared dynamic
// vertex buffer, fetch the swap chain's back buffer to read its format
// (GetDesc), create a matching depth-stencil surface, then — unless
// render-to-texture is disabled (DAT_00cf1998) or disabled by a
// per-mode capability flag at compute_dynamic_table_size()+0x3c —
// create an offscreen render-target texture of the same format and
// switch rendering to its level-0 surface. Finally (always) bind the
// render target and depth-stencil surface and refill the default render
// state profile.
void prepare_render_target_texture() {
    reset_dynamic_resources();
    if (DAT_00c75a7c == nullptr) {
        create_dynamic_vertex_buffer();
    }
    reset_hook_placeholder();

    using GetBackBufferCall =
        std::int32_t (__stdcall*)(void*, std::uint32_t, std::uint32_t, void*);
    (*reinterpret_cast<GetBackBufferCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1948)) + 0x14))(
        DAT_00cf1948, 0, 0, &DAT_00cf1954);
    DAT_00bb0a40 = DAT_00cf1954;

    std::uint32_t surface_desc[8] = {}; // D3DSURFACE_DESC; [0] = Format
    using GetDescCall = std::int32_t (__stdcall*)(void*, void*);
    (*reinterpret_cast<GetDescCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00bb0a40)) + 0x30))(
        DAT_00bb0a40, surface_desc);

    using CreateDepthStencilSurfaceCall = std::int32_t (__stdcall*)(
        void*, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t,
        std::uint32_t, std::uint32_t, void*, std::uint32_t);
    (*reinterpret_cast<CreateDepthStencilSurfaceCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x74))(
        DAT_00cf1944, DAT_008663b0, DAT_008663b4, DAT_00b170b8, 0, 0, 0,
        &DAT_00cf195c, 0);

    if (DAT_00cf1998 != 0) {
        DAT_00cf1958 = reinterpret_cast<void*>(static_cast<std::uintptr_t>(0xbacb0ffeu));
    } else if (DAT_008663d4 != 0 && DAT_00865287 != 0) {
        const auto* capability_record = reinterpret_cast<const std::uint8_t*>(
            static_cast<std::uintptr_t>(compute_dynamic_table_size()));
        const auto flags = *reinterpret_cast<const std::uint32_t*>(capability_record + 0x3c);
        const bool skip_texture = (flags & 0x100) == 0 && (flags & 2) != 0;
        if (!skip_texture) {
            using CreateTextureCall = std::int32_t (__stdcall*)(
                void*, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t,
                std::uint32_t, std::uint32_t, void*, std::uint32_t);
            void* texture = nullptr;
            const auto hr = (*reinterpret_cast<CreateTextureCall const*>(
                static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x5c))(
                DAT_00cf1944, DAT_008663b0, DAT_008663b4, 1, 1, surface_desc[0], 0,
                &texture, 0);
            if (hr >= 0) {
                using ReleaseCall = void (__stdcall*)(void*);
                (*reinterpret_cast<ReleaseCall const*>(
                    static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1954)) + 8))(
                    DAT_00cf1954);

                using GetSurfaceLevelCall =
                    std::int32_t (__stdcall*)(void*, std::uint32_t, void*);
                (*reinterpret_cast<GetSurfaceLevelCall const*>(
                    static_cast<const std::uint8_t*>(vtable_of(texture)) + 0x48))(
                    texture, 0, &DAT_00bb0a40);

                DAT_00cf1958 = texture;
                DAT_00cf1954 = DAT_00bb0a40;
            }
        }
    }

    DAT_00ba14f8 = DAT_00cf195c;
    using SetRenderTargetCall = std::int32_t (__stdcall*)(void*, std::uint32_t, void*);
    (*reinterpret_cast<SetRenderTargetCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x94))(
        DAT_00cf1944, 0, DAT_00bb0a40);

    using SetDepthStencilSurfaceCall = std::int32_t (__stdcall*)(void*, void*);
    (*reinterpret_cast<SetDepthStencilSurfaceCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x9c))(
        DAT_00cf1944, DAT_00ba14f8);

    initialize_default_render_state();
}

} // namespace hp6::render
