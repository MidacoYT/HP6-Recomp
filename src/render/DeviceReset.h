#pragma once
#include <cstdint>

namespace hp6::render {

// 0x00650d80. Bare prologue/epilogue, no body — a placeholder reset hook.
void reset_hook_placeholder();

// 0x0063b4a0. Pure arithmetic: DAT_008663d8 * 0x584 + 0x454 + DAT_00cf19d0.
// Looks like a per-entry-stride table size (0x584 bytes/entry) plus a
// fixed header and an extra offset; exact purpose unresolved.
std::int32_t compute_dynamic_table_size();

// 0x00648e10. A single Direct3D 9 vtable call on the device (DAT_00cf1944)
// at offset 0x68 (CreateVertexBuffer, index 26) with Length=0xe0,
// Usage=(DAT_00cf19d4 | 0x208), FVF=0, Pool=(DAT_00865286 != 0 ? 0 : 2),
// ppVertexBuffer=&DAT_00c75a7c, pSharedHandle=0. The HRESULT is not
// checked, matching the exported x86. Every D3D9 vtable call in this
// module pushes the device/object pointer on the stack rather than
// passing it through ECX (STDMETHODCALLTYPE is __stdcall on x86), unlike
// this project's other, non-COM virtual calls, which are __thiscall.
void create_dynamic_vertex_buffer();

// 0x006525d0. Releases (vtable+8) each of the DAT_00cf1a30 vertex shaders
// held in the DAT_00a68820 table (field_4 = shader object), then
// recreates all of them via the device's CreateVertexShader (vtable+0x16c,
// index 91) with field_0 as the bytecode pointer and &field_4 as the
// output slot.
void recreate_vertex_shaders();

// 0x006534a0 (selector is read but never changes behavior; ReAgent's own
// candidate added a spurious extra pointer indirection here, flagged by
// its own objective reviewer and confirmed wrong against the exported
// x86 — this reads the vtable and the method pointer directly, two loads
// total, then calls Release(), vtable+8). Releases `shader` if present.
void release_shader_if_present(std::int32_t selector, void* shader);

// 0x00653430. When `shader_type` is 0, (re)creates a vertex shader
// (device vtable+0x16c) from `bytecode` into *(record+0x2c). Otherwise,
// if the ushort at record+0x22 equals DAT_00cf1a38, clears *(record+0x2c)
// to 0 instead of recreating; else creates a pixel shader (device
// vtable+0x1a8, CreatePixelShader, index 106) the same way.
void recreate_or_invalidate_shader(std::uint32_t bytecode,
                                   std::uint32_t unused_param_2,
                                   std::int32_t shader_type,
                                   std::int32_t record_address);

// 0x006532e0. Walks the fixed 0x600-entry, 0xc4-byte-stride table at
// DAT_00f38b10. For every entry whose word at +0x18 (the bytecode
// pointer) is non-zero: a first pass releases the shader object at +0x2c
// (release_shader_if_present, with the type flag at +0x14 as the unused
// selector); a second pass calls recreate_or_invalidate_shader with
// (bytecode = +0x18, unused = +0x1c, shader_type = +0x14, record = the
// entry's own address).
void recreate_shader_table();

// 0x00642420. Always calls recreate_vertex_shaders(); when DAT_00cf19bc
// is greater than 2, also (tail-calls, in the exported x86)
// recreate_shader_table().
void reset_dynamic_resources();

// 0x0064aa30. Direct3D 9 GetAvailableTextureMem (device vtable+0x10,
// index 4, no arguments beyond the device) gives the free texture memory
// in bytes; shifted right by 0x14 (divide by ~1 MiB) and cached in
// DAT_00cf1964. When that cached value is non-zero and at most 0x20
// (i.e. 1-32 MiB free), calls adjust_texture_quality_for_memory.
void cache_available_texture_memory();

// 0x0063b4c0 / 0x01011a50. Recomputes DAT_008663dc (a texture/LOD scale
// used elsewhere) from the cached available-memory value
// (DAT_00cf1964), a per-mode capability flag at mode+0x474 bit 0x800,
// and the escalation counter DAT_00cf19bc; DAT_00cf1974, when non-zero,
// overrides the result outright.
void adjust_texture_quality_for_memory();

} // namespace hp6::render
