#include "render/RenderStateCache.h"

#include <cstring>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern void* DAT_00cf1944; // IDirect3DDevice9*
extern std::int32_t DAT_00c75188[]; // SetRenderState cache, one word per state
extern std::int32_t DAT_009968d8[]; // SetTextureStageState cache, [stage*0x20+type]

extern std::uint32_t DAT_00cf18c4;
extern std::uint32_t DAT_00cf18c8;
extern std::int32_t DAT_00cf18cc;
extern std::int32_t DAT_00cf18e0;
// The 19-word render-state profile buffer. apply_render_state_profile's
// own write targets (indices 1-18, i.e. DAT_00881374..DAT_008813b8 in the
// exported globals) are the SAME fixed addresses as this buffer;
// initialize_default_render_state is the only caller reconstructed so
// far, and it always passes this exact array.
extern std::uint32_t DAT_00881370[19];
extern std::uint32_t DAT_008813bc; // trailing word, just past the profile buffer
extern std::uint32_t DAT_008813c0;
extern std::uint32_t DAT_008813c4;
extern std::int32_t DAT_008663d8;      // active display mode index
extern std::int32_t DAT_00cf19d0;      // display mode table base
extern std::uint32_t DAT_00cf19a8;     // active pixel format (matches DeviceCreate.cpp's own extern type)
extern std::int32_t DAT_0086527c;
extern std::int32_t DAT_00865280;

extern std::int32_t DAT_00b108a0[]; // SetSamplerState cache, [sampler*0x20+type]
// SetTexture cache, one slot per stage (also, words 0-3 only, zeroed by
// hp6::render::clear_scratch_light_block — see DeviceCreate.cpp).
extern std::uint32_t DAT_00bb09b4[16];
extern void* DAT_00cf19c4;          // SetVertexDeclaration cache
extern std::uint32_t DAT_008663cc;  // SetVertexShader cache (handle, stored as a plain word)
extern std::uint32_t DAT_008663d0;  // SetPixelShader cache (handle, stored as a plain word)
extern std::uint32_t DAT_0096b484;  // vertex-shader-constant cache word, reset alongside the shader cache
extern std::uint32_t DAT_00bac98c;  // vertex-shader-constant cache word, reset alongside the shader cache

extern std::uint8_t DAT_00a70890[]; // vertex-buffer-handle table (handle >= 0), 0x14-byte stride
extern std::uint8_t DAT_00baa170[]; // vertex-buffer-handle table (handle < 0), 0x14-byte stride
extern std::uint8_t DAT_0098c6a8[]; // index-buffer-handle table (handle >= 0), 0x14-byte stride
extern std::uint8_t DAT_009985a0[]; // index-buffer-handle table (handle < 0), 0x14-byte stride
extern std::int32_t DAT_00998580;   // the active render region's mode index (set by set_active_render_region)
extern std::int32_t DAT_00c754cc;   // highest valid mode index in the per-mode capability tables
extern std::uint8_t DAT_00f38b28[]; // per-mode capability table, 0xc4-byte stride (also read by FUN_005e8e70, not yet integrated)
extern std::int32_t DAT_00cf1a3c;   // fallback mode index when the active one is out of range
extern std::int32_t DAT_00cf1a48;   // bind_pixel_shader_for_mode's own cache
extern std::uint32_t DAT_00b170c4;  // bind_pixel_shader_for_mode's rebind counter

extern std::uint32_t DAT_00f82c40;
extern std::uint32_t DAT_00f82c44;
extern std::uint32_t DAT_00f82c48;
extern std::uint32_t DAT_00f82c8c;
extern std::uint32_t DAT_00f82c90;
extern std::uint32_t DAT_00f82c94;
extern std::uint32_t DAT_00f82cd8;
extern std::uint32_t DAT_00f82cdc;
extern std::uint32_t DAT_00f82ce0;
extern std::uint32_t DAT_00f82d24;
extern std::uint32_t DAT_00f82d28;
extern std::uint32_t DAT_00f82d2c;
extern std::uint32_t DAT_00f82d70;
extern std::uint32_t DAT_00f82d74;
extern std::uint32_t DAT_00f82d78;
extern std::uint32_t DAT_00f82dbc;
extern std::uint32_t DAT_00f82dc0;
extern std::uint32_t DAT_00f82dc4;
extern std::uint32_t DAT_00f82e08;
extern std::uint32_t DAT_00f82e0c;
extern std::uint32_t DAT_00f82e10;
extern std::uint32_t DAT_00f82e54;
extern std::uint32_t DAT_00f82e58;
extern std::uint32_t DAT_00f82e5c;
extern std::uint32_t DAT_00f82ea0;
extern std::uint32_t DAT_00f82ea4;
extern std::uint32_t DAT_00f82ea8;
extern std::uint32_t DAT_00f82ecc;
extern std::uint32_t DAT_00f82eec;
extern std::uint32_t DAT_00f82ef0;
extern std::uint32_t DAT_00f82ef4;
extern std::uint32_t DAT_00f82f38;
extern std::uint32_t DAT_00f82f3c;
extern std::uint32_t DAT_00f82f40;
extern std::uint32_t DAT_00f82f84;
extern std::uint32_t DAT_00f82f88;
extern std::uint32_t DAT_00f82f8c;
extern std::uint32_t DAT_00f82fd0;
extern std::uint32_t DAT_00f82fd4;
extern std::uint32_t DAT_00f82fd8;
extern std::uint32_t DAT_00f8301c;
extern std::uint32_t DAT_00f83020;
extern std::uint32_t DAT_00f83024;
extern std::uint32_t DAT_00f83068;
extern std::uint32_t DAT_00f8306c;
extern std::uint32_t DAT_00f83070;
extern std::uint32_t DAT_00f830b4;
extern std::uint32_t DAT_00f830b8;
extern std::uint32_t DAT_00f830bc;
extern std::uint32_t DAT_00f83100;
extern std::uint32_t DAT_00f83104;
extern std::uint32_t DAT_00f83108;
extern std::uint32_t DAT_00f8314c;
extern std::uint32_t DAT_00f83150;
extern std::uint32_t DAT_00f83154;
extern std::uint32_t DAT_00f83198;
extern std::uint32_t DAT_00f8319c;
extern std::uint32_t DAT_00f831a0;
extern std::uint32_t DAT_00f831e4;
extern std::uint32_t DAT_00f831e8;
extern std::uint32_t DAT_00f831ec;
extern std::uint32_t DAT_00f83230;
extern std::uint32_t DAT_00f83234;
extern std::uint32_t DAT_00f83238;
extern std::uint32_t DAT_00f8327c;
extern std::uint32_t DAT_00f83280;
extern std::uint32_t DAT_00f83284;
extern std::uint32_t DAT_00f832c8;
extern std::uint32_t DAT_00f832cc;
extern std::uint32_t DAT_00f832d0;
extern std::uint32_t DAT_00f83314;
extern std::uint32_t DAT_00f83318;
extern std::uint32_t DAT_00f8331c;
extern std::uint32_t DAT_00f83340;
extern std::uint32_t DAT_00f83360;
extern std::uint32_t DAT_00f83364;
extern std::uint32_t DAT_00f83368;
extern std::uint32_t DAT_00f8338c;
extern std::uint32_t DAT_00f833ac;
extern std::uint32_t DAT_00f833b0;
extern std::uint32_t DAT_00f833b4;
extern std::uint32_t DAT_00f833d8;
extern std::uint32_t DAT_00f83494;
extern std::uint32_t DAT_00f83498;
extern std::uint32_t DAT_00f834a0;
extern std::uint32_t DAT_00f834a4;
extern std::uint32_t DAT_00f834ac;
extern std::uint32_t DAT_00f834b0;
extern std::uint32_t DAT_00f834b8;
extern std::uint32_t DAT_00f834bc;
extern std::uint32_t DAT_00f834c4;
extern std::uint32_t DAT_00f834c8;
extern std::uint32_t DAT_00f834d0;
extern std::uint32_t DAT_00f834d4;
extern std::uint32_t DAT_00f834dc;
extern std::uint32_t DAT_00f834e0;
extern std::uint32_t DAT_00f834e8;
extern std::uint32_t DAT_00f834ec;
extern std::uint32_t DAT_00f834f4;
extern std::uint32_t DAT_00f834f8;
extern std::uint32_t DAT_00f83500;
extern std::uint32_t DAT_00f83504;
extern std::uint32_t DAT_00f8350c;
extern std::uint32_t DAT_00f83510;

namespace hp6::render {
namespace {

#ifdef _MSC_VER
using D3DCall2 = std::int32_t (__stdcall*)(void*, std::int32_t, std::int32_t);
using D3DCall3 = std::int32_t (__stdcall*)(void*, std::int32_t, std::int32_t, std::int32_t);
using D3DCallSetTexture = std::int32_t (__stdcall*)(void*, std::int32_t, void*);
using D3DCallSetHandle = std::int32_t (__stdcall*)(void*, void*);
using D3DCallSetVsConstF = std::int32_t (__stdcall*)(void*, std::uint32_t, const void*, std::uint32_t);
using D3DCallSetStreamSource = std::int32_t (__stdcall*)(void*, std::uint32_t, void*, std::uint32_t, std::uint32_t);
using D3DCallSetIndices = std::int32_t (__stdcall*)(void*, void*);
#else
using D3DCall2 = std::int32_t (*)(void*, std::int32_t, std::int32_t);
using D3DCall3 = std::int32_t (*)(void*, std::int32_t, std::int32_t, std::int32_t);
using D3DCallSetTexture = std::int32_t (*)(void*, std::int32_t, void*);
using D3DCallSetHandle = std::int32_t (*)(void*, void*);
using D3DCallSetVsConstF = std::int32_t (*)(void*, std::uint32_t, const void*, std::uint32_t);
using D3DCallSetStreamSource = std::int32_t (*)(void*, std::uint32_t, void*, std::uint32_t, std::uint32_t);
using D3DCallSetIndices = std::int32_t (*)(void*, void*);
#endif

void* vtable_of(void* object) {
    return *reinterpret_cast<void* const*>(object);
}

} // namespace

// ReAgent candidate: reports/pipeline/candidates/0064c300.cpp. Corrected
// from __thiscall to __stdcall (see src/render/DeviceReset.cpp): the
// exported x86 pushes the device pointer on the stack before the call.
std::uint32_t set_render_state_cached(std::int32_t state, std::int32_t value) {
    std::uint32_t result = 0;
    if (DAT_00c75188[state] != value) {
        auto call = *reinterpret_cast<D3DCall2 const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0xe4);
        result = static_cast<std::uint32_t>(call(DAT_00cf1944, state, value));
        DAT_00c75188[state] = value;
    }
    return result;
}

// ReAgent candidate: reports/pipeline/rejected/0064c3b0.cpp, with the
// same __stdcall correction. The cache index (stage * 0x20 + type) is
// mathematically identical to the exported x86's byte-offset arithmetic
// (stage * 0x80 + type * 4, divided by 4); ReAgent's reviewer flagged the
// rewritten form as a possible signed-overflow concern, but stage/type
// are small render-pipeline indices here, far from overflow.
std::uint32_t set_texture_stage_state_cached(std::int32_t stage, std::int32_t type,
                                             std::int32_t value) {
    std::uint32_t result = 0;
    auto& cached = DAT_009968d8[stage * 0x20 + type];
    if (cached != value) {
        auto call = *reinterpret_cast<D3DCall3 const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x10c);
        result = static_cast<std::uint32_t>(call(DAT_00cf1944, stage, type, value));
        cached = value;
    }
    return result;
}

// 0x010021f0 (ESI calling convention; see header). Reconstructed from the
// exported x86: ReAgent's candidates for this address and its thunk
// (0x006406a0) both treated the ESI-sourced pointer as the unresolved
// `unaff_ESI`; the only caller reconstructed so far
// (initialize_default_render_state) always passes its own freshly
// prepared DAT_00881370 buffer, which this function then both reads from
// and writes back into unchanged — confirmed harmless for that call site
// and faithfully reproduced for any other caller found later.
void apply_render_state_profile(const std::uint32_t* settings) {
    set_render_state_cached(0x13, static_cast<std::int32_t>(settings[1]));
    set_render_state_cached(0x14, static_cast<std::int32_t>(settings[2]));
    set_render_state_cached(0x1b, static_cast<std::int32_t>(settings[3]));
    set_render_state_cached(0x80, static_cast<std::int32_t>(settings[4]));
    set_render_state_cached(0x81, static_cast<std::int32_t>(settings[5]));
    set_render_state_cached(0x82, static_cast<std::int32_t>(settings[6]));
    set_render_state_cached(0x83, static_cast<std::int32_t>(settings[7]));
    set_render_state_cached(0x84, static_cast<std::int32_t>(settings[8]));
    set_render_state_cached(0x85, static_cast<std::int32_t>(settings[9]));
    set_render_state_cached(0x86, static_cast<std::int32_t>(settings[10]));
    set_render_state_cached(0x87, static_cast<std::int32_t>(settings[11]));
    set_render_state_cached(0xab, static_cast<std::int32_t>(settings[12]));
    set_texture_stage_state_cached(0, 1, static_cast<std::int32_t>(settings[13]));
    set_texture_stage_state_cached(0, 2, static_cast<std::int32_t>(settings[14]));
    set_texture_stage_state_cached(0, 3, static_cast<std::int32_t>(settings[15]));
    set_texture_stage_state_cached(0, 4, static_cast<std::int32_t>(settings[16]));
    set_texture_stage_state_cached(0, 5, static_cast<std::int32_t>(settings[17]));
    set_texture_stage_state_cached(0, 6, static_cast<std::int32_t>(settings[18]));

    DAT_00881370[1] = settings[1];
    DAT_00881370[2] = settings[2];
    DAT_00881370[3] = settings[3];
    DAT_00881370[4] = settings[4];
    DAT_00881370[5] = settings[5];
    DAT_00881370[6] = settings[6];
    DAT_00881370[7] = settings[7];
    DAT_00881370[8] = settings[8];
    DAT_00881370[9] = settings[9];
    DAT_00881370[10] = settings[10];
    DAT_00881370[11] = settings[11];
    DAT_00881370[12] = settings[12];
    DAT_00881370[13] = settings[13];
    DAT_00881370[14] = settings[14];
    DAT_00881370[15] = settings[15];
    DAT_00881370[16] = settings[16];
    DAT_00881370[17] = settings[17];
    DAT_00881370[18] = settings[18];
    DAT_0086527c = static_cast<std::int32_t>(settings[0]);
}

// ReAgent candidate: reports/pipeline/candidates/00640fb0.cpp, verified
// assignment-by-assignment (143 of 143) against the exported decompiled
// C. Reconstructed call to the ESI-passing 0x010021f0 made explicit as
// apply_render_state_profile(DAT_00881370).
void initialize_default_render_state() {
    DAT_00cf18c4 = 0;
    DAT_00cf18c8 = 0;
    DAT_00cf18cc = 0;
    DAT_00cf18e0 = 0;
    std::memset(DAT_00881370, 0xcd, sizeof DAT_00881370);

    DAT_00f82c90 = 6;
    DAT_00f82cdc = 6;
    DAT_00f82d74 = 8;
    DAT_00f82dc0 = 8;
    DAT_00f83068 = 6;
    DAT_00f830b4 = 6;
    DAT_00f83100 = 8;
    DAT_00f8314c = 8;
    DAT_00f82c48 = 0;
    DAT_00f82c40 = 2;
    DAT_00f82c44 = 1;
    DAT_00f82c94 = 1;
    DAT_00f82c8c = 5;
    DAT_00f82ce0 = 1;
    DAT_00f82cd8 = 5;
    DAT_00f82d24 = 2;
    DAT_00f82d28 = 1;
    DAT_00f82d70 = 7;
    DAT_00f82dbc = 7;
    DAT_00f82e10 = 1;
    DAT_00f82e08 = 5;
    DAT_00f82e0c = 2;
    DAT_00f82e54 = 7;
    DAT_00f82ea8 = 1;
    DAT_00f82ea0 = 5;
    DAT_00f82ea4 = 2;
    DAT_00f82ecc = 3;
    DAT_00f82eec = 7;
    DAT_00f82ef0 = 1;
    DAT_00f82f40 = 1;
    DAT_00f82f38 = 5;
    DAT_00f82f3c = 7;
    DAT_00f82f8c = 1;
    DAT_00f82f84 = 5;
    DAT_00f82f88 = 2;
    DAT_00f82fd0 = 7;
    DAT_00f83024 = 0;
    DAT_00f8301c = 2;
    DAT_00f83020 = 1;
    DAT_00f83070 = 1;
    DAT_00f8306c = 5;
    DAT_00f830bc = 1;
    DAT_00f830b8 = 5;
    DAT_00f83104 = 7;
    DAT_00f83150 = 7;
    DAT_00f831a0 = 0;
    DAT_00f83198 = 9;
    DAT_00f8319c = 3;
    DAT_00f831ec = 0;
    DAT_00f831e8 = 3;
    DAT_00f8338c = 3;
    DAT_00f831e4 = 9;
    DAT_00f83238 = 1;
    DAT_00f83230 = 2;
    DAT_00f83234 = 5;
    DAT_00f83284 = 1;
    DAT_00f8327c = 2;
    DAT_00f83280 = 5;
    DAT_00f832d0 = 1;
    DAT_00f832c8 = 1;
    DAT_00f832cc = 2;
    DAT_00f8331c = 0;
    DAT_00f83314 = 2;
    DAT_00f83318 = 2;
    DAT_00f83340 = 2;
    DAT_00f83368 = 0;
    DAT_00f83360 = 2;
    DAT_00f83364 = 2;
    DAT_00f833b4 = 0;
    DAT_00f833ac = 2;
    DAT_00f833b0 = 2;
    DAT_00f833d8 = 1;

    if (((*reinterpret_cast<std::uint8_t*>(
              static_cast<std::uintptr_t>(DAT_00cf19d0) +
              DAT_008663d8 * 0x584 + 0x474)) &
          0x80) == 0 ||
        (DAT_00cf19a8 == 0x10)) {
        DAT_00f82d24 = 1;
        DAT_00f82d28 = 2;
        DAT_00f82d70 = 1;
        DAT_00f82d74 = 2;
        DAT_00f82dbc = 1;
        DAT_00f82dc0 = 2;
        DAT_00f82e54 = 1;
        DAT_00f82eec = 1;
        DAT_00f82ef0 = 2;
        DAT_00f82fd0 = 1;
        DAT_00f83100 = 1;
        DAT_00f83104 = 2;
        DAT_00f8314c = 1;
        DAT_00f83150 = 2;
    }

    DAT_00f83154 = 1;
    DAT_00f83108 = 1;
    DAT_00f82fd8 = 1;
    DAT_00f82fd4 = 2;
    DAT_00f82ef4 = 1;
    DAT_00f82e5c = 1;
    DAT_00f82e58 = 2;
    DAT_00f82dc4 = 1;
    DAT_00f82d78 = 1;
    DAT_00f82d2c = 1;
    DAT_008813bc = 0xcdcdcdcdu;
    DAT_008813c0 = 0xcdcdcdcdu;
    DAT_008813c4 = 0xcdcdcdcdu;

    DAT_00f834e0 = 5;
    DAT_00881370[13] = 5;
    DAT_00f83494 = 0;
    DAT_00f83498 = 8;
    DAT_00f834a0 = 1;
    DAT_00f834a4 = 8;
    DAT_00f834ac = 1;
    DAT_00f834b0 = 2;
    DAT_00f834b8 = 1;
    DAT_00f834bc = 4;
    DAT_00f834c4 = 1;
    DAT_00f834c8 = 3;
    DAT_00f834d0 = 1;
    DAT_00f834d4 = 7;
    DAT_00f834dc = 1;
    DAT_00f834e8 = 1;
    DAT_00f834ec = 6;
    DAT_00f834f4 = 1;
    DAT_00f834f8 = 7;
    DAT_00f83500 = 1;
    DAT_00f83504 = 7;
    DAT_00f8350c = 1;
    DAT_00f83510 = 7;

    DAT_00881370[1] = 2;
    DAT_00881370[2] = 1;
    DAT_00881370[3] = 0;
    DAT_00881370[4] = 0;
    DAT_00881370[5] = 0;
    DAT_00881370[6] = 0;
    DAT_00881370[7] = 0;
    DAT_00881370[12] = 1;
    DAT_00881370[15] = 2;
    DAT_00881370[14] = 0;
    DAT_00881370[16] = 4;
    DAT_00881370[18] = 2;
    DAT_00881370[17] = 0;

    apply_render_state_profile(DAT_00881370);
    DAT_008813c0 = 0;
    DAT_008813c4 = 8;
    set_render_state_cached(0x19, 8);
    DAT_0086527c = -8;
    DAT_00865280 = -7;
}

// 0x0064c350 (cdecl). Direct3D 9 SetSamplerState (device vtable+0x114,
// index 69) with a per-(sampler,type) cache at
// DAT_00b108a0[sampler*0x20+type], same shape as
// set_texture_stage_state_cached above.
std::uint32_t set_sampler_state_cached(std::int32_t sampler, std::int32_t type,
                                       std::int32_t value) {
    std::uint32_t result = 0;
    auto& cached = DAT_00b108a0[sampler * 0x20 + type];
    if (cached != value) {
        auto call = *reinterpret_cast<D3DCall3 const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x114);
        result = static_cast<std::uint32_t>(call(DAT_00cf1944, sampler, type, value));
        cached = value;
    }
    return result;
}

// 0x0064c410 (cdecl). Direct3D 9 SetTexture (device vtable+0x104, index
// 65) with a per-stage cache at DAT_00bb09b4[stage] (the handle stored
// as a plain uint32, matching the exported x86 and this file's other
// handle caches below).
std::uint32_t set_texture_cached(std::int32_t stage, void* texture) {
    std::uint32_t result = 0;
    const auto handle = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(texture));
    if (DAT_00bb09b4[stage] != handle) {
        auto call = *reinterpret_cast<D3DCallSetTexture const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x104);
        result = static_cast<std::uint32_t>(call(DAT_00cf1944, stage, texture));
        DAT_00bb09b4[stage] = handle;
    }
    return result;
}

// 0x0064c5c0 (cdecl). Direct3D 9 SetVertexDeclaration (device
// vtable+0x15c, index 87) with a cache at DAT_00cf19c4.
std::uint32_t set_vertex_declaration_cached(void* declaration) {
    std::uint32_t result = 0;
    if (DAT_00cf19c4 != declaration) {
        auto call = *reinterpret_cast<D3DCallSetHandle const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x15c);
        result = static_cast<std::uint32_t>(call(DAT_00cf1944, declaration));
        DAT_00cf19c4 = declaration;
    }
    return result;
}

// 0x006524b0. Resets the two vertex-shader-constant cache words the
// exported x86 keeps alongside the vertex-shader cache; invalidated
// whenever set_vertex_shader_cached actually installs a new (resolved)
// shader handle.
void reset_vertex_shader_constant_cache() {
    DAT_0096b484 = 0;
    DAT_00bac98c = 0;
}

// 0x0064c4e0 (cdecl). Direct3D 9 SetVertexShader (device vtable+0x170,
// index 92) with a cache at DAT_008663cc. When `resolve` is set, the
// exported x86 first passes `handle` through 0x01003de0 (NOT YET
// INTEGRATED: not traced, so the handle is forwarded unchanged here —
// a documented no-op that keeps the call shape faithful without
// fabricating the real resolution logic) and invalidates the
// vertex-shader-constant cache afterward.
std::uint32_t set_vertex_shader_cached(void* handle, bool resolve) {
    std::uint32_t result = 0;
    const auto handle_word = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(handle));
    if (DAT_008663cc != handle_word) {
        // NOT YET INTEGRATED: when `resolve` is set, the exported x86
        // first passes `handle` through thunk_FUN_01003de0 — not
        // traced, so the handle is forwarded unchanged here.
        auto call = *reinterpret_cast<D3DCallSetHandle const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x170);
        result = static_cast<std::uint32_t>(call(DAT_00cf1944, handle));
        if (resolve) {
            reset_vertex_shader_constant_cache();
        }
        DAT_008663cc = handle_word;
    }
    return result;
}

// 0x0064c570 (cdecl). Direct3D 9 SetPixelShader (device vtable+0x1ac,
// index 107) with a cache at DAT_008663d0.
std::uint32_t set_pixel_shader_cached(void* handle) {
    std::uint32_t result = 0;
    const auto handle_word = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(handle));
    if (DAT_008663d0 != handle_word) {
        auto call = *reinterpret_cast<D3DCallSetHandle const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x1ac);
        result = static_cast<std::uint32_t>(call(DAT_00cf1944, handle));
        DAT_008663d0 = handle_word;
    }
    return result;
}

void set_vertex_shader_constant_f(std::int32_t start_register, const void* data,
                                  std::int32_t vector4_count) {
    auto call = *reinterpret_cast<D3DCallSetVsConstF const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x178);
    call(DAT_00cf1944, static_cast<std::uint32_t>(start_register), data,
         static_cast<std::uint32_t>(vector4_count));
}

void bind_vertex_buffer_by_handle(std::uint32_t handle, std::uint32_t stride,
                                  std::uint32_t offset, std::uint32_t stream_number) {
    if (handle == 0xbad1bad1u) {
        return;
    }
    const std::uint8_t* table = (static_cast<std::int32_t>(handle) >= 0) ? DAT_00a70890 : DAT_00baa170;
    const std::uint32_t slot = handle & 0x7fffffffu;
    void* buffer = *reinterpret_cast<void* const*>(table + slot * 0x14 + 0x10);
    auto call = *reinterpret_cast<D3DCallSetStreamSource const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 400);
    call(DAT_00cf1944, stream_number, buffer, offset, stride);
}

void bind_index_buffer_by_handle(std::uint32_t handle) {
    if (handle == 0xbad1bad1u) {
        return;
    }
    const std::uint8_t* table = (static_cast<std::int32_t>(handle) >= 0) ? DAT_0098c6a8 : DAT_009985a0;
    const std::uint32_t slot = handle & 0x7fffffffu;
    void* indices = *reinterpret_cast<void* const*>(table + slot * 0x14 + 0x10);
    auto call = *reinterpret_cast<D3DCallSetIndices const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x1a0);
    call(DAT_00cf1944, indices);
}

void* resolve_mode_pixel_shader_handle() {
    std::int32_t mode = DAT_00998580;
    const std::uint8_t* entry = DAT_00f38b28 + static_cast<std::size_t>(mode) * 0xc4;
    if (mode > DAT_00c754cc || *reinterpret_cast<const std::int32_t*>(entry) == 0) {
        mode = DAT_00cf1a3c;
    }
    const std::uint8_t* resolved = DAT_00f38b28 + static_cast<std::size_t>(mode) * 0xc4;
    return *reinterpret_cast<void* const*>(resolved + 0x14); // DAT_00f38b3c's own offset from DAT_00f38b28
}

void bind_pixel_shader_for_mode(std::int32_t handle, bool force) {
    if (DAT_00cf1a48 != handle || force) {
        void* resolved = resolve_mode_pixel_shader_handle();
        set_pixel_shader_cached(resolved);
        DAT_00cf1a48 = handle;
        ++DAT_00b170c4;
    }
}

} // namespace hp6::render
