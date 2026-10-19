#pragma once
#include <cstdint>

namespace hp6::render {

// 0x00646200 ("Scene::End" per its own embedded diagnostic string,
// "Scene::End - warning drawprim/draw indexed failed?!?!\n"). The real
// per-batch draw executor at the end of this project's now fully traced
// UI/scene render chain (see architecture/startup_path.md, "iMsgDoRender
// / GUI rendering trail", suite 14): PluginPresentationBitmap_
// BuildRenderQuad builds a quad+text descriptor, which flows through a
// clip/layout step and a material-bucketed command queue
// (hp6::render::set_active_render_region's own region slots) down to
// this function, called once per queued draw item by the real per-frame
// flush (FUN_00647480, not yet integrated).
//
// `context` is a cursor into a parallel per-draw material/colour stream
// (advanced by 0x40/0xc0-byte strides as multi-item batches recurse);
// `batch` is the draw item itself (or, when its first byte is 'M', a
// list of sub-batches to recurse into — each entry a {pointer, extra
// flags} pair, 8 bytes, count at batch+2); `flags` carries per-item
// render flags; `region_index`/`region_table_entry` identify the active
// render region.
//
// Three real, distinct code paths, all faithfully reproduced:
//   1. flags bit 0x80000000 set: not a draw at all — invalidates up to
//      8 cached declaration slots chained from region_table_entry+0x1c
//      (stride 0xc), by setting bit 0x4000 of each one's own +0x26
//      field, then clears the region's last-bound-material cache
//      (DAT_00cf18ec).
//   2. batch's first byte is 'M': a multi-item list — recurses once per
//      entry, advancing `context`'s own cursor fields between calls.
//   3. otherwise, a single draw item: picks between two sibling code
//      paths depending on DAT_008663d5 (the already-integrated quality-
//      tier flag) — a fuller material/lighting-bound path and a
//      simpler fallback — each ending in the real Direct3D 9 draw call:
//      DrawPrimitive (device vtable+0x144, index 81) when the item has
//      no index buffer (index-buffer field == the 0xbad1bad1 "none"
//      sentinel), else DrawIndexedPrimitive (vtable+0x148, index 82).
//      Both call sites' arguments are read directly from the draw
//      item's own fixed fields (type/start-vertex/base-vertex/
//      vertex-count/start-index/primitive-count), not from anything
//      the deferred material-binding blocks below compute — so the
//      real draw call's own parameters are faithful regardless of what
//      is deferred.
//
// The low-tier path's own material/lighting setup is now fully real:
// push_item_transform_and_material (FUN_006455c0, transform + colour
// constants) and resolve_material_bindings (FUN_00644cd0's second code
// path, vertex declaration/shader bind + "actually draw" gate) below.
// NOT YET INTEGRATED, documented deferred no-ops: the fuller
// (DAT_008663d5 == 0) path's own equivalent material/lighting setup —
// thunk_FUN_01002c30, a texture-coordinate-scale constant push,
// FUN_00644cd0's FIRST code path (a separate lighting-flags cache), and
// FUN_00642380's declaration-handle resolution — all large, untraced
// functions whose output only affects how that path's draw looks, never
// whether or with what primitive parameters the real draw call itself
// fires.
void execute_scene_render_batch(std::int32_t* context, std::uint8_t* batch, std::uint32_t flags,
                                std::int32_t region_index, std::uint8_t* region_table_entry);

// 0x006455c0 (cdecl). Pushes the active transform and material/colour
// vertex-shader constants for the low-tier draw path above: when the
// item's transform pointer (context[0], a 16-float row-major matrix)
// changed since the last call, multiplies it by the active render
// region's own view/projection matrix (one of two fixed per-region
// tables selected by context[1]'s bit 0x40000000, indexed by
// DAT_00cf1914 -- the same active-region index
// hp6::render::execute_scene_render_batch's own caller,
// FUN_00647480, is not yet integrated but already confirmed to write),
// transposes the result, and uploads it via the already-integrated
// hp6::render::set_vertex_shader_constant_f at register 0. Then pushes
// up to three colour/material constant groups (registers 4, 5, 6,
// 0x12) and the final blend-ish render state (0x16), all gated by
// context[1]'s own flag bits, reusing set_vertex_shader_constant_f and
// the already-integrated hp6::render::set_render_state_cached
// throughout -- every sub-call this function makes is now a real,
// traced Direct3D 9 operation; none of it is deferred.
void push_item_transform_and_material(std::int32_t* context);

// 0x00644cd0's second code path (reached with its own `force_material_
// dispatch` parameter set -- the exported x86's first path, reached
// with that parameter clear, drives a separate lighting-flags cache
// not needed by the low-tier draw path above and is NOT reconstructed
// here). Sets hp6::render::execute_scene_render_batch's own "actually
// draw" gate (DAT_00865285) to true, then switches on `material_type`
// to bind the real vertex declaration + vertex shader pair for a
// handful of known material types via the already-integrated
// set_vertex_declaration_cached/set_vertex_shader_cached (including
// 0x20000108, the same 2D-overlay material
// hp6::render::render_fade_overlay_quad already uses
// DAT_00cf1930/DAT_00cf192c for -- confirming both real draw-call
// integrations in this project share the same real material). Every
// other recognized material type dispatches through
// hp6::render::resolve_material_shader_effect (0x01002960 -- despite
// the "thunk_" name other code in this project refers to it by, it is
// a real, now-integrated function: see its own doc comment in
// SceneBatchRenderer.cpp). Only a truly unrecognized material_type clears the gate,
// matching the exported x86's own default case.
void resolve_material_bindings(std::uint32_t material_type);

} // namespace hp6::render
