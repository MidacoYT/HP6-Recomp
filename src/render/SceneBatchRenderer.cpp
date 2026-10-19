#include "render/SceneBatchRenderer.h"
#include "render/RenderStateCache.h"
#include "math/Transform.h"
#include "platform/Diagnostics.h"

#include <cstddef>
#include <cstdint>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern void* DAT_00cf1944;        // IDirect3DDevice9*
extern std::uint8_t DAT_008663d5; // quality-tier flag (already integrated elsewhere)
extern std::uint32_t DAT_00b10ce4; // per-call-depth bookkeeping counter
extern std::uint32_t DAT_009968c4; // multi-batch item-count accumulator
extern std::int32_t DAT_00cf18ec;  // region's last-bound-material cache, cleared on invalidate

extern std::uint32_t DAT_00866358; // last-bound vertex buffer handle cache (also used elsewhere)
extern std::uint32_t DAT_00cf1908; // last-bound vertex stride cache (also used elsewhere)
extern std::uint32_t DAT_0086635c; // last-bound index buffer handle cache
extern std::uint32_t DAT_00866360; // last-bound material-flags cache (also used elsewhere)
extern std::uint32_t DAT_00ba0cf0; // low-tier-path stream rebind counter

extern std::uint8_t DAT_00cf1895;  // "collect draw stats" flag
extern std::uint32_t DAT_009968ac; // draw-call stat counter
extern std::uint32_t DAT_009968b0, DAT_009968b4, DAT_009968b8;
extern std::uint32_t DAT_009968c8, DAT_009968cc;

extern std::uint8_t DAT_00865285; // low-tier path's own "actually draw" gate

// push_item_transform_and_material's (0x006455c0) own extern dependencies.
extern std::int32_t DAT_00c76680;   // previous call's flags, written only (read by other, not-yet-integrated code)
extern std::uint32_t DAT_00ba14fc;  // previous item's flags, shared with FUN_00644cd0's deferred lighting branch
extern std::int32_t DAT_00c76684;   // last transform pointer (context[0]) processed
extern std::uint8_t DAT_00f82530[]; // per-region view/projection matrix table (flags bit 0x40000000 clear); 0x40-byte stride, 16 row-major floats each
extern std::uint8_t DAT_00f82310[]; // per-region view/projection matrix table (flags bit 0x40000000 set); same stride/shape
extern std::int32_t DAT_00cf1914;   // active render region index (written by FUN_00647480, not yet integrated)
extern std::uint32_t DAT_0099858c;  // transform-recompute counter
extern std::int32_t DAT_00c76698;   // last context[6] (texture-projection matrix) pointer processed
extern std::int32_t DAT_00c7668c;   // last context[5] (texture/light array) pointer processed
extern std::uint32_t DAT_00c766a8;  // register-4 colour constant cache (sentinel 0x80808080, else the raw colour dword)
extern std::int32_t DAT_00c76690;   // last context[5] pointer bound directly to register 4
extern std::uint32_t DAT_00c766ac;  // register-6 colour constant cache, same shape as DAT_00c766a8
extern float DAT_00848478;          // fixed default colour component
extern float DAT_007db234;          // fixed colour-channel clamp/max (also the alpha scale's clamp ceiling)
extern float DAT_00831c2c;          // fixed colour-channel default (commonly 0)
extern float DAT_008483dc;          // colour-channel scale, register-4 path
extern float DAT_008483f0;          // colour-channel scale, register-6/alpha path

// resolve_material_bindings's (0x00644cd0's second code path) own extern
// dependencies.
extern float DAT_00848418;  // fixed fog/material default colour component
extern void* DAT_00cf1934;  // vertex declaration handle, material type 2
extern void* DAT_00cf1924;  // vertex shader handle, material type 2
extern void* DAT_0098c69c;  // vertex declaration handle, material type 10
extern void* DAT_00cf1928;  // vertex shader handle, material type 10
extern void* DAT_00cf1930;  // vertex declaration handle, material type 0x20000108 (shared with ScreenOverlayRender.cpp)
extern void* DAT_00cf192c;  // vertex shader handle, material type 0x20000108 (shared with ScreenOverlayRender.cpp)

// resolve_material_shader_effect's (0x01002960) own extern dependencies.
// DAT_008a6750/DAT_008a6478 are flat, lazily-populated declaration/shader
// handle caches indexed by the same `variant*0xe + group*0x1816c + index`
// arithmetic the exported x86 uses; their true bounds are not needed since
// every access goes through that same formula.
extern void* DAT_008a6750[]; // cached vertex declaration handle per (variant, group, index)
extern void* DAT_008a6478[]; // cached vertex shader handle per (variant, group, index)

#ifdef _MSC_VER
using D3DCallDrawPrimitive = std::int32_t(__stdcall*)(void*, std::uint32_t, std::uint32_t, std::uint32_t);
using D3DCallDrawIndexedPrimitive = std::int32_t(__stdcall*)(void*, std::uint32_t, std::int32_t,
                                                             std::uint32_t, std::uint32_t,
                                                             std::uint32_t, std::uint32_t);
#else
using D3DCallDrawPrimitive = std::int32_t(*)(void*, std::uint32_t, std::uint32_t, std::uint32_t);
using D3DCallDrawIndexedPrimitive = std::int32_t(*)(void*, std::uint32_t, std::int32_t, std::uint32_t,
                                                    std::uint32_t, std::uint32_t, std::uint32_t);
#endif

namespace hp6::render {
namespace {

constexpr std::uint32_t kNoBufferSentinel = 0xbad1bad1u;

void* vtable_of(void* object) {
    return *reinterpret_cast<void* const*>(object);
}

std::uint16_t read_u16(const std::uint8_t* p, std::ptrdiff_t offset) {
    return *reinterpret_cast<const std::uint16_t*>(p + offset);
}
std::uint32_t read_u32(const std::uint8_t* p, std::ptrdiff_t offset) {
    return *reinterpret_cast<const std::uint32_t*>(p + offset);
}

// Real Direct3D 9 draw calls (device vtable+0x144/0x148, indices 81/82),
// with every argument read directly from the draw item's own fixed
// fields -- identical regardless of what material-binding state has or
// has not been set up beforehand.
std::int32_t draw_item(const std::uint8_t* item) {
    const std::uint32_t primitive_type = item[0x25] & 0x7fu;
    const std::uint32_t start_vertex = read_u16(item, 0x20);
    const std::uint32_t primitive_count = read_u16(item, 0x1e);
    const std::uint32_t index_buffer_handle = read_u32(item, 0x14);

    if (index_buffer_handle == kNoBufferSentinel) {
        auto draw = *reinterpret_cast<D3DCallDrawPrimitive const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x144);
        return draw(DAT_00cf1944, primitive_type, start_vertex, primitive_count);
    }
    auto draw_indexed = *reinterpret_cast<D3DCallDrawIndexedPrimitive const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x148);
    return draw_indexed(DAT_00cf1944, primitive_type, static_cast<std::int32_t>(start_vertex), 0,
                        read_u16(item, 0x26), read_u16(item, 0x1c), primitive_count);
}

void accumulate_draw_stats(std::uint32_t flags, const std::uint8_t* item) {
    if (DAT_00cf1895 == 0) {
        return;
    }
    ++DAT_009968ac;
    DAT_009968b0 += (flags & 1u) != 0u;
    DAT_009968b4 += (flags & 0x10u) == 0u;
    DAT_009968b8 += (flags & 4u) != 0u;
    DAT_009968c8 += read_u16(item, 4);
    DAT_009968cc += read_u16(item, 2);
}

// 0x01002960 (despite the "thunk_" name other code refers to it by, this
// is a real function, not a thunk). Resolves a shader effect by (group,
// index) for material types without one of the fixed declaration/shader
// pairs resolve_material_bindings binds directly below. Picks a "variant"
// index (0..0xc) from DAT_00ba14fc's lighting-flag bits and
// `material_type`'s own bits, then either reuses the plain default
// material (DAT_00cf1934/DAT_00cf1924, same as material type 2) when the
// variant is the trivial one, or looks up a lazily-populated per-
// (variant, group, index) declaration/shader handle cache
// (DAT_008a6750/DAT_008a6478).
//
// NOT YET INTEGRATED: when a cache slot is empty, the exported x86 calls
// FUN_00641f70 to build a real D3DVERTEXELEMENT9 array from
// `material_type`'s feature bits -- its own decompiled body has an
// unresolved implicit register input (`in_EAX`) and an `unaff_EBX` Ghidra
// warning, signalling a real ambiguity that would need the same kind of
// live-disassembly work hp6::math::transpose_matrix4x4's callers
// required, not yet done. Left as a documented no-op (a null
// declaration/shader handle) rather than guessing; the real
// set_vertex_declaration_cached/set_vertex_shader_cached calls below
// still fire with that placeholder handle, so this never changes whether
// resolve_material_bindings's own "actually draw" gate (DAT_00865285)
// stays open.
void resolve_material_shader_effect(std::int32_t group, std::int32_t index, std::uint32_t material_type) {
    std::int32_t variant;
    if ((DAT_00ba14fc & 4u) == 0u) {
        if ((material_type & 0x3000u) != 0u && (material_type & 8u) == 0u) {
            group = 1;
        }
        if ((DAT_00ba14fc & 0x100u) == 0u) {
            if ((DAT_00ba14fc & 0x200u) == 0u) {
                if ((DAT_00ba14fc & 0x80u) == 0u) {
                    variant = (DAT_00ba14fc & 0x40u) == 0u ? 0 : 9;
                } else {
                    variant = (material_type & 0x3000u) == 0u ? 0 : 3;
                }
            } else {
                variant = (material_type & 0x3000u) == 0u ? 0 : 2;
            }
        } else {
            variant = 0xb;
        }
        if ((DAT_00ba14fc & 0x10000u) != 0u) {
            if (variant == 0) {
                variant = 1;
            } else if (variant == 9) {
                variant = 10;
            } else {
                if (variant != 0xb) {
                    hp6::platform::format_diagnostic_message(
                        "MAXZ variant requested on a shader which has no support for this yet\n");
                }
                variant = 0xc;
            }
        }
    } else if ((DAT_00ba14fc & 0x100u) == 0u) {
        if ((DAT_00ba14fc & 0x200u) == 0u) {
            if ((DAT_00ba14fc & 0x80u) == 0u) {
                if ((DAT_00ba14fc & 0x40u) == 0u) {
                    variant = (material_type & 0x3000u) == 0u ? 0 : 4;
                } else {
                    variant = (material_type & 0x3000u) == 0u ? 9 : 7;
                }
            } else {
                variant = (material_type & 0x3000u) == 0u ? 0 : 3;
            }
        } else {
            variant = (material_type & 0x3000u) == 0u ? 0 : 2;
        }
    } else {
        variant = (material_type & 0x3000u) == 0u ? 0xb : 8;
    }

    if (variant == 0 && (material_type & 0x110u) == 0u) {
        set_vertex_declaration_cached(DAT_00cf1934);
        set_vertex_shader_cached(DAT_00cf1924, true);
        return;
    }

    const std::size_t cache_index = static_cast<std::size_t>(variant) * 0xeu +
                                    static_cast<std::size_t>(group) * 0x1816cu +
                                    static_cast<std::size_t>(index);
    if (DAT_008a6750[cache_index] == nullptr) {
        // NOT YET INTEGRATED: see the function-level comment above.
        DAT_008a6750[cache_index] = nullptr;
    }
    set_vertex_declaration_cached(DAT_008a6750[cache_index]);
    set_vertex_shader_cached(DAT_008a6478[cache_index], true);
}

} // namespace

void push_item_transform_and_material(std::int32_t* context) {
    DAT_00c76680 = static_cast<std::int32_t>(DAT_00ba14fc);
    DAT_00ba14fc = static_cast<std::uint32_t>(context[1]);
    std::uint32_t flags = static_cast<std::uint32_t>(context[1]);

    if (context[0] != DAT_00c76684) {
        const auto* transform = reinterpret_cast<const float*>(static_cast<std::intptr_t>(context[0]));
        const std::uint8_t* table = (flags & 0x40000000u) == 0u ? DAT_00f82530 : DAT_00f82310;
        const auto* region_matrix =
            reinterpret_cast<const float*>(table + static_cast<std::size_t>(DAT_00cf1914) * 0x40u);

        float multiplied[16];
        hp6::math::multiply_matrix4x4(multiplied, transform, region_matrix);
        float transposed[16];
        hp6::math::transpose_matrix4x4(transposed, multiplied);
        set_vertex_shader_constant_f(0, transposed, 4);

        DAT_00c76684 = context[0];
        ++DAT_0099858c;
    }

    std::uint32_t colour = static_cast<std::uint32_t>(context[4]);
    if ((flags & 0x3c0u) != 0u) {
        if ((flags & 0x280u) != 0u) {
            colour = 0x80808080u;
            flags = (flags & 0xffffeffbu) | 8u;
        }
        if (context[6] != DAT_00c76698) {
            const auto* projection = reinterpret_cast<const float*>(static_cast<std::intptr_t>(context[6]));
            float transposed_projection[16];
            hp6::math::transpose_matrix4x4(transposed_projection, projection);
            set_vertex_shader_constant_f(0x12, transposed_projection, 4);
            DAT_00c76698 = context[6];
        }
    }

    if ((flags & 4u) != 0u && DAT_00c7668c != context[5]) {
        set_vertex_shader_constant_f(
            8, reinterpret_cast<const void*>(static_cast<std::intptr_t>(context[5])), 9);
        DAT_00c7668c = context[5];
    }

    if ((flags & 8u) == 0u) {
        if ((flags & 0x1000u) == 0u) {
            float group4[4] = {DAT_00848478, DAT_00848478, DAT_00848478, DAT_007db234};
            set_vertex_shader_constant_f(4, group4, 1);
            DAT_00c766a8 = 0x80808080u;

            float group5[4] = {DAT_007db234, DAT_00831c2c, DAT_00831c2c, DAT_00831c2c};
            set_vertex_shader_constant_f(5, group5, 1);
        } else {
            if (DAT_00c76690 != context[5]) {
                set_vertex_shader_constant_f(
                    4, reinterpret_cast<const void*>(static_cast<std::intptr_t>(context[5])), 1);
                DAT_00c76690 = context[5];
            }
            float group5[4] = {DAT_007db234, DAT_007db234, DAT_00831c2c, DAT_00831c2c};
            set_vertex_shader_constant_f(5, group5, 1);
        }
    } else {
        if (colour != DAT_00c766a8) {
            float group4[4];
            group4[0] = static_cast<float>(colour & 0xffu) * DAT_008483dc;
            group4[1] = static_cast<float>((colour >> 8) & 0xffu) * DAT_008483dc;
            group4[2] = static_cast<float>((colour >> 0x10) & 0xffu) * DAT_008483dc;
            float alpha = static_cast<float>(colour >> 0x18) * DAT_008483f0;
            if (DAT_007db234 < alpha) {
                alpha = DAT_007db234;
            }
            group4[3] = alpha;
            set_vertex_shader_constant_f(4, group4, 1);
            DAT_00c766a8 = colour;
        }
        float group5[4] = {DAT_00831c2c, DAT_007db234, DAT_00831c2c, DAT_00831c2c};
        set_vertex_shader_constant_f(5, group5, 1);
    }

    if ((flags & 0x800u) == 0u) {
        if (DAT_00c766ac != 0x80808080u) {
            float group6[4] = {DAT_007db234, DAT_007db234, DAT_007db234, DAT_007db234};
            set_vertex_shader_constant_f(6, group6, 1);
            DAT_00c766ac = 0x80808080u;
        }
    } else if (colour != DAT_00c766ac) {
        float group6[4];
        group6[0] = static_cast<float>(colour & 0xffu) * DAT_008483f0;
        group6[1] = static_cast<float>((colour >> 8) & 0xffu) * DAT_008483f0;
        group6[2] = static_cast<float>((colour >> 0x10) & 0xffu) * DAT_008483f0;
        float alpha = static_cast<float>(colour >> 0x18) * DAT_008483f0;
        if (DAT_007db234 < alpha) {
            alpha = DAT_007db234;
        }
        group6[3] = alpha;
        set_vertex_shader_constant_f(6, group6, 1);
        DAT_00c766ac = colour;
    }

    if ((flags & 0x20u) == 0u) {
        set_render_state_cached(0x16, (flags & 0x400u) == 0u ? 3 : 2);
    } else {
        set_render_state_cached(0x16, 1);
    }
}

void resolve_material_bindings(std::uint32_t material_type) {
    DAT_00865285 = 1;

    if ((DAT_00866360 & 0x110u) != (material_type & 0x110u)) {
        float group[4];
        if ((material_type & 0x10u) == 0u) {
            group[0] = group[1] = group[2] = group[3] = DAT_007db234;
        } else {
            group[0] = group[1] = DAT_00848418;
            group[2] = group[3] = DAT_007db234;
        }
        set_vertex_shader_constant_f(0x11, group, 1);
    }

    const std::uint32_t masked = material_type & 0xfffdffffu;
    bool recognized = true;

    if (masked < 0x101bu) {
        if (masked == 0x101au) {
            resolve_material_shader_effect(0, 2, 0x101a);
        } else if (masked < 0x103u) {
            if (masked == 0x102u) {
                resolve_material_shader_effect(1, 0xb, 0x102);
            } else switch (masked) {
                case 2:
                    set_vertex_declaration_cached(DAT_00cf1934);
                    set_vertex_shader_cached(DAT_00cf1924, true);
                    break;
                case 10:
                    set_vertex_declaration_cached(DAT_0098c69c);
                    set_vertex_shader_cached(DAT_00cf1928, true);
                    break;
                case 0x12:
                    resolve_material_shader_effect(1, 10, masked);
                    break;
                case 0x1a:
                    resolve_material_shader_effect(0, 8, masked);
                    break;
                default:
                    recognized = false;
                    break;
            }
        } else if (masked == 0x10au) {
            resolve_material_shader_effect(0, 9, 0x10a);
        } else if (masked == 0x1002u) {
            resolve_material_shader_effect(1, 6, 0x1002);
        } else if (masked == 0x1012u) {
            resolve_material_shader_effect(1, 6, 0x1012);
        } else {
            recognized = false;
        }
    } else if (masked < 0x201bu) {
        if (masked == 0x201au) {
            resolve_material_shader_effect(0, 0, 0x201a);
        } else if (masked < 0x2003u) {
            if (masked == 0x2002u) {
                resolve_material_shader_effect(1, 4, 0x2002);
            } else if (masked == 0x1102u) {
                resolve_material_shader_effect(1, 7, 0x1102);
            } else if (masked == 0x110au) {
                resolve_material_shader_effect(0, 3, 0x110a);
            } else {
                recognized = false;
            }
        } else if (masked == 0x2012u) {
            resolve_material_shader_effect(1, 4, 0x2012);
        } else {
            recognized = false;
        }
    } else {
        if (masked == 0x2102u) {
            resolve_material_shader_effect(1, 5, 0x2102);
        } else if (masked == 0x210au) {
            resolve_material_shader_effect(0, 1, 0x210a);
        } else if (masked == 0x20000108u) {
            set_vertex_declaration_cached(DAT_00cf1930);
            set_vertex_shader_cached(DAT_00cf192c, true);
        } else {
            recognized = false;
        }
    }

    // The exported x86 calls the confirmed true no-op FUN_00405a30
    // (`{ return; }`) whenever one of the real calls above reports a
    // non-zero HRESULT; omitted here, matching this project's established
    // convention for other confirmed no-op calls.
    if (!recognized) {
        DAT_00865285 = 0;
    }
}

void execute_scene_render_batch(std::int32_t* context, std::uint8_t* batch, std::uint32_t flags,
                                std::int32_t region_index, std::uint8_t* region_table_entry) {
    if ((flags & 0x80000000u) != 0u) {
        // Not a draw: invalidate up to 8 cached declaration slots chained
        // from the region's own table entry, matching the exported x86's
        // own bit-0x4000 "dirty" marker on each slot's +0x26 field.
        std::uint32_t pending = read_u32(region_table_entry, 0x14) & 0x3ffu;
        for (int slot = 0; slot < 8; ++slot) {
            if ((pending & 1u) != 0u) {
                auto* node = *reinterpret_cast<std::uint8_t* const*>(region_table_entry + 0x1c + slot * 0xc);
                auto* flags_field = reinterpret_cast<std::uint16_t*>(node + 0x26);
                *flags_field = (*flags_field & 0x3fffu) | 0x4000u;
            }
            pending >>= 1u;
        }
        DAT_00cf18ec = 0;
        return;
    }

    if (batch == nullptr) {
        return;
    }

    ++DAT_00b10ce4;
    std::int32_t result = 0;

    if (batch[0] == 'M') {
        // Multi-item batch: each entry is {pointer, extra flags}, 8 bytes,
        // count at batch+2. Recurse once per entry, advancing `context`'s
        // own cursor fields the same way the exported x86 does between
        // calls.
        const std::uint16_t count = read_u16(batch, 2);
        const std::uint8_t* cursor = batch + 0x10;
        for (std::uint16_t i = 0; i < count; ++i) {
            auto* entry_batch = *reinterpret_cast<std::uint8_t* const*>(cursor);
            const std::uint32_t entry_flags = read_u32(cursor, 4);
            cursor += 8;
            execute_scene_render_batch(context, entry_batch, flags | entry_flags, region_index,
                                       region_table_entry);
            context[0] += 0x40;
            if ((flags & 0x3c0u) != 0u) {
                context[6] += 0x40;
            }
            if (DAT_008663d5 == 0 && (flags & 0x1004u) != 0u) {
                context[5] += 0xc0;
            }
        }
        DAT_009968c4 += count;
        return;
    }

    if (DAT_008663d5 == 0) {
        // NOT YET INTEGRATED: the exported x86's own material/lighting
        // setup here -- thunk_FUN_01002c30, a texture-coordinate-scale
        // constant push, FUN_00644cd0 (render-flag computation), and
        // FUN_00642380's declaration-handle resolution -- all deferred
        // as documented no-ops. None of it affects the real draw call's
        // own parameters below, only how the draw looks.
        if (read_u16(batch, 0x1e) != 0) {
            const std::uint32_t vertex_buffer_handle = read_u32(batch, 0x10);
            if (vertex_buffer_handle != kNoBufferSentinel) {
                // NOT YET INTEGRATED: FUN_00644cd0/FUN_00642380's
                // declaration-lookup half (the vertex-stream binding
                // itself, bind_vertex_streams_for_material, is not
                // reconstructed here since its own declaration-handle
                // resolution, FUN_00641ba0/FUN_00642170, is untraced).
                const std::uint32_t index_buffer_handle = read_u32(batch, 0x14);
                if (index_buffer_handle != kNoBufferSentinel && DAT_0086635c != index_buffer_handle) {
                    bind_index_buffer_by_handle(index_buffer_handle);
                    DAT_0086635c = index_buffer_handle;
                }
                result = draw_item(batch);
                accumulate_draw_stats(flags, batch);
            }
        }
        // Only the fuller (DAT_008663d5 == 0) path checks the draw
        // result and caches the item's own material-flags byte
        // (batch+8) into DAT_00866360 -- the exported x86's low-tier
        // sibling below does its own, separate DAT_00866360 write from
        // a different field (batch+0x18) and never checks `result`.
        if (result != 0) {
            hp6::platform::format_diagnostic_message(
                "Scene::End - warning drawprim/draw indexed failed?!?!\n");
        }
        DAT_00866360 = read_u32(batch, 8);
    } else {
        set_pixel_shader_cached(nullptr);
        push_item_transform_and_material(context);
        if (read_u16(batch, 0x1e) != 0) {
            const std::uint32_t vertex_buffer_handle = read_u32(batch, 0x10);
            const std::uint8_t declaration_index = batch[0x24];
            if (DAT_00866358 != vertex_buffer_handle || DAT_00cf1908 != declaration_index) {
                bind_vertex_buffer_by_handle(vertex_buffer_handle, declaration_index, 0, 0);
                DAT_00cf1908 = declaration_index;
                ++DAT_00ba0cf0;
                DAT_00866358 = vertex_buffer_handle;
            }
            const std::uint32_t index_buffer_handle = read_u32(batch, 0x14);
            if (index_buffer_handle != kNoBufferSentinel && DAT_0086635c != index_buffer_handle) {
                bind_index_buffer_by_handle(index_buffer_handle);
                DAT_0086635c = index_buffer_handle;
            }
            // resolve_material_bindings reads the PREVIOUS material-flags
            // cache (DAT_00866360) to decide whether register 0x11 needs
            // rewriting, so it must run before that cache is updated to
            // this item's own value -- exactly the order the exported
            // x86 uses.
            const std::uint32_t material_type = read_u32(batch, 0x18);
            resolve_material_bindings(material_type);
            DAT_00866360 = material_type;
            if (DAT_00865285 != 0 && vertex_buffer_handle != kNoBufferSentinel) {
                (void)draw_item(batch);
                accumulate_draw_stats(flags, batch);
            }
        }
    }

    (void)region_index;
    ++DAT_00b10ce4;
}

} // namespace hp6::render
