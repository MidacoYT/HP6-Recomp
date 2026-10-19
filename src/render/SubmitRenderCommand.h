#pragma once
#include <cstdint>

namespace hp6::render {

// 0x00542e10 (cdecl; confirmed via live disassembly, not the decompiled
// C alone, which mislabels `bucket` as if it indexed a flat global array
// -- it actually indexes *within* the active render region's own block).
// "SubmitRenderCommand" -- 62 real, non-vtable-indirect callers across
// the whole engine, the busiest single real function this project has
// found so far. Queues `command` (a variable-length record: 0x10 bytes
// when none of its own flag bits 0xbfff7bcf are set, else 0x24 bytes --
// the SAME two-shape distinction hp6::render::execute_scene_render_batch
// already draws between its 0x10-byte-stride multi-batch entries and its
// larger single-item records) into a singly-linked list (next-pointer at
// the queued item's own +0xc) selected by `bucket`.
//
// Two real routing modes, both read from the bucket's own region
// descriptor (DAT_009970d8[bucket], a pointer this function does not
// otherwise interpret beyond its +0x14 mode dword and +0x22 word):
//   - Normal mode (+0x14 & 0xc0000000 != 0x40000000): queues into the
//     ACTIVE render region's own per-bucket array
//     (hp6::render::set_active_render_region's own DAT_00967218, at
//     +bucket*4 for the list tail, +0x8000+bucket*4 for the list head --
//     the same 0x10000-byte-stride region block, confirming it holds
//     both arrays back to back).
//   - "Indexed" mode (+0x14 & 0xc0000000 == 0x40000000): queues into a
//     GLOBAL (not per-region) per-material-subtype bucket instead
//     (_Dst_00b10ce8 for the tail, _Dst_00966fd8 for the head, both
//     indexed by (+0x14 >> 10) & 0xff), and appends 3 extra fields after
//     the copied payload (+0x24: `bucket` itself; +0x26: the command's
//     own flags shifted right by 0xf; +0x28: DAT_00998580, the active
//     render-region's own mode -- already read elsewhere in this
//     project's RenderStateCache.cpp) -- a real, observable 0x2c-byte
//     item size in this mode vs. the plain 0x10/0x24 in normal mode.
//
// Either way, marks the bucket as touched by the active region
// (DAT_0098c690[bucket] |= DAT_0089cbfe, the SAME region-mask byte
// hp6::render::set_active_render_region already writes) and a
// per-region-descriptor byte (DAT_009978e0[region_descriptor's own
// +0x22 word] = 0x3f) unconditionally, then advances the shared bump
// write-pointer (DAT_0096b48c) by the real item size.
//
// NOT YET INTEGRATED: when `command`'s own bit 0 is set, the exported
// x86 calls thunk_FUN_010026a0 (a large DAT_008663d5-quality-tier-gated
// material/shader dispatch with several more untraced sub-calls) and may
// clear that bit based on its result -- left as a documented deferred
// no-op (the bit is never cleared here), matching this project's
// established doctrine for appearance-only sub-dispatches that don't
// change whether or with what size/shape the real command gets queued.
void submit_render_command(std::int16_t bucket, std::int32_t* command);

} // namespace hp6::render
