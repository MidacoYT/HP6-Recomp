// hp6::render::GeometryBatcher — reconstruction of the shared batched
//-geometry pipeline the engine uses for every dynamic geometry submission
// (37 callers: 0x006511f0/0x006512f0/0x006516f0 appear across the whole
// presentation and render code).
//
// Evidence trail (binary disassembly / decompile of each function):
//   0x006511f0          thin jump-thunk to 0x01003b20 (begin a batch)
//   0x01003b20          begin_vertex_batch: parse the 0x30-byte batch
//                       command's +0x18 vertex-format mask, compute the
//                       real per-vertex stride (0x01003ce0), stash the
//                       active command in the DAT_00cf1a10..DAT_00cf1a24
//                       register file, and either reset the scratch
//                       vertices/indices cursors (first batch) or reserve
//                       space by locking the scratch buffers (0x00650f70).
//   0x01003ce0          compute_vertex_format_stride (see the one-to-one
//                       tables in its C++ port below).
//   0x006516f0          write_batch_vertices: copy `count` 0x40-byte
//                       source records into the active destination
//                       buffer, packing fields according to the format
//                       mask bits described in 0x01003ce0's port.
//   0x006512f0          flush_vertex_batch: copy the accumulated scratch
//                       vertices/indices into the locked vertex/index
//                       buffer(s) (whose data pointers the 00650f70 lock
//                       produced), compute the final vertex/index counts,
//                       unlock static-pool buffers still marked locked
//                       (vtable +0x30 on the D3D9 buffer object), reset the
//                       batch registers, and clear the "pending" byte
//                       0x00cf19de.
//   0x00650f70          lock_scratch_buffers: acquire a scratch vertex
//                       buffer slot (0x00651d60) and, when the batch uses
//                       indices, a scratch index buffer slot (0x00651f80),
//                       lock each with vtable +0x2c (Lock on
//                       IDirect3DVertexBuffer9 / IDirect3DIndexBuffer9:
//                       flags 0x800 = D3DLOCK_NOOVERWRITE,
//                       0x2000 = D3DLOCK_DISCARD when the batch asked for
//                       a discard-mode buffer through DAT_00cf1a1c), and
//                       expose the write cursors through
//                       DAT_00cf1a00/DAT_00cf1a08.
//   0x00651d60          acquire_scratch_vertex_buffer: pick a free entry in
//                       the per-stride vertex-buffer pools
//                       (DAT_00a70890 entries; handle bit 31 = dynamic)
//                       or create a new one with vtable+0x68 on the D3D
//                       device DAT_00cf1944.
//   0x00651f80          acquire_scratch_index_buffer: 0x20000-byte index
//                       pools (DAT_0098c6a8 entries), created with
//                       vtable+0x6c.
//
// Like every other reconstruction here, this header's contract follows the
// observable byte/word/dword layout of the original commands; abstractions
// above that contract (e.g. what a "handle" means to D3D9) are named only
// where the x86 names them.
#pragma once

#include <cstdint>
#include <cstddef>

namespace hp6::render {

// The original 0x30-byte batch command record, byte-exactly as FUN_005e9820
// (hp6::presentation::TextLayout.cpp's submit_text_runs) provisions it and
// the batcher consumes it:
struct BatchCommand {
    // 0x0000: type markers written by the callers' emit path
    // (0x4850 'P','H' in the text path); read/owned outside this batcher.
    std::uint8_t type_0;
    std::uint8_t type_1;
    std::uint8_t pad_2[0x6];
    // 0x0008: dword written alongside type markers (0x12 in the text path)
    std::uint32_t marker_8;
    std::uint8_t pad_0xc[0xc];
    // 0x0018: on entry = the requested vertex-format mask; begin_vertex_batch
    // replaces it with the computed per-vertex byte stride.
    std::uint32_t format_or_stride_18;
    // 0x001c: u16, begin_vertex_batch zeroes it for index-less batches.
    std::uint16_t word_1c;
    // 0x001e: u16, callers accumulate per emitted run.
    std::uint16_t word_1e;
    // 0x0020/0x0021: untouched here.
    std::uint8_t pad_20[2];
    // 0x0022: u16, index count of the batch (read by flush_vertex_batch).
    std::uint16_t index_count_22;
    // 0x0024: begin_vertex_batch stores the low byte of the vertex stride.
    std::uint8_t vertex_stride_24;
    // 0x0025: caller-owned byte (0x4 in the text path).
    std::uint8_t caller_flag_25;
    // 0x0026: u16, batched vertex count; flush_vertex_batch computes and
    // stores it when zero (from the DAT_00cf19f0..DAT_00cf19f8 cursor span).
    std::uint16_t vertex_count_26;
    std::uint8_t pad_28[0x8];
};
static_assert(sizeof(BatchCommand) == 0x30, "original command is 0x30 bytes");
static_assert(offsetof(BatchCommand, word_1c) == 0x1c, "field offset");
static_assert(offsetof(BatchCommand, word_1e) == 0x1e, "field offset");
static_assert(offsetof(BatchCommand, index_count_22) == 0x22, "field offset");
static_assert(offsetof(BatchCommand, vertex_stride_24) == 0x24, "field offset");
static_assert(offsetof(BatchCommand, caller_flag_25) == 0x25, "field offset");
static_assert(offsetof(BatchCommand, vertex_count_26) == 0x26, "field offset");
static_assert(offsetof(BatchCommand, format_or_stride_18) == 0x18, "field offset");

// 0x01003b20 (entered through the 0x006511f0 jump-thunk in the original).
// `format` is the vertex-format bit mask the command carried at +0x18,
// `mode` is the batch's usage mode (bit values drive Lock flags and
// buffer pools through DAT_00cf1a1c), `index_mode` selects index handling
// (bit 0 set = "no index buffer"; convention unchanged from the original).
std::uint8_t begin_vertex_batch(BatchCommand* command, std::uint32_t mode,
                                std::uint32_t index_mode);

// 0x01003ce0. Returns the stride in `vertex_stride` and preserves the
// (unchanged) axis mask in `axis_mask`, matching the original's out-params.
void compute_vertex_format_stride(std::uint32_t format,
                                  std::uint32_t* axis_mask,
                                  std::uint32_t* vertex_stride);

// 0x006516f0. `source` points at `count` consecutive 0x40-byte records in
// the packed layout the original reads through dword indices 0..15.
void write_batch_vertices(const void* source, std::uint32_t count);

// 0x006512f0. Copies the current scratch batch to D3D9 buffer pools,
// finalizes counts and handles, and clears the pending batch registers.
void flush_vertex_batch();

// ---- Not yet integrated (declaration only; no translation unit defines
// ---- them, mirroring the project's deferred-block doctrine) ---------------
// Other geometry paths that enter 0x00650f70 from a non-fresh batch still
// need a dedicated verification pass before begin_vertex_batch can route
// that branch through the pool implementation.

} // namespace hp6::render
