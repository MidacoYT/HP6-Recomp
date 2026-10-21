#include "render/GeometryBatcher.h"

#include <cstring>

// Batch register file + scratch storage, all with the exact roles the
// exported x86 gives them. DAT_00cf1a08's declared type mirrors
// runtime/ScratchBuffers.cpp (pointer to a cursor variable holding a byte
// address); DAT_00cf18e4 is consumed by the command's callers
// (presentation/TextLayout.cpp), not here.
extern void* DAT_00cf19f0;      // scratch vertices base
extern void* DAT_00cf19f4;      // scratch indices base
extern void* DAT_00cf19f8;      // scratch vertices cursor (rewound here)
extern void* DAT_00cf19fc;      // scratch indices cursor (rewound here)
extern void** DAT_00cf1a00;     // -> the scratch vertices cursor variable
extern void* _DAT_00cf1a04;     // vertices mirror slot (written, unread here)
extern std::uint32_t* DAT_00cf1a08; // -> the scratch indices cursor variable
extern void* _DAT_00cf1a0c;     // indices mirror slot (written, unread here)
extern std::uint32_t DAT_00cf1a10;  // active vertex-format mask
extern std::uint32_t DAT_00cf1a14;  // active computed per-vertex stride
extern std::uint32_t DAT_00cf1a18;  // active per-vertex stride
extern std::uint32_t DAT_00cf1a1c;  // active batch usage mode
extern void* DAT_00cf1a20;      // active BatchCommand
extern std::uint32_t DAT_00cf1a24;  // active batch index_mode
extern std::uint32_t DAT_00cf19de;  // "batch pending" byte (result mirror)
extern std::uint8_t DAT_008663d5;   // low-quality-mode flag (DeviceCreate.cpp)

namespace hp6::render {

// 0x01003ce0 ----------------------------------------------------------------
// One-to-one with the exported x86:
//   bit 0x2    -> +0xc (3-float position, source dwords 0..2)
//   bit 0x1000 -> +0xc (source dwords 7..9)
//   bit 0x8    -> +0x4 (source dword 6)
//   axis slots 0..3 (bits 0x10/0x20/0x40/0x80 each = +0x4; otherwise its
//     0x100-wide companion bit 0x1xx = +0x8 over dwords 4..5)
//   bit 0x10000 -> +0xc (source dwords 10..12)
//   bit 0x8000  -> +0xc (source dwords 13..15)
//   bit 0x20000 -> +8 or +4 by the DAT_008663d5 low-quality flag (the value
//     itself is beyond this function's evidence).
void compute_vertex_format_stride(std::uint32_t format,
                                  std::uint32_t* axis_mask,
                                  std::uint32_t* vertex_stride) {
    std::uint32_t stride = 0;
    if ((format & 0x2) != 0) {
        stride += 0xc;
    }
    if ((format & 0x1000) != 0) {
        stride += 0xc;
    }
    if ((format & 0x8) != 0) {
        stride += 0x4;
    }
    for (std::uint32_t slot = 0; slot < 4; ++slot) {
        if ((format & (0x10u << slot)) != 0) {
            stride += 0x4;
        } else if ((format & (0x100u << slot)) != 0) {
            stride += 0x8;
        }
    }
    if ((format & 0x10000) != 0) {
        stride += 0xc;
    }
    if ((format & 0x8000) != 0) {
        stride += 0xc;
    }
    if ((format & 0x20000) != 0) {
        stride += (DAT_008663d5 == 0) ? 8 : 4;
    }
    *axis_mask = format;
    *vertex_stride = stride;
}

// 0x006511f0 -> 0x01003b20 (0x006511f0 is a pure jump-thunk into
// 0x01003b20, so one reconstruction covers both addresses) -------------------
std::uint8_t begin_vertex_batch(BatchCommand* command, std::uint32_t mode,
                                std::uint32_t index_mode) {
    DAT_00cf1a10 = command->format_or_stride_18;
    compute_vertex_format_stride(DAT_00cf1a10, &DAT_00cf1a14, &DAT_00cf1a18);
    command->format_or_stride_18 = DAT_00cf1a14;
    command->vertex_stride_24 = static_cast<std::uint8_t>(DAT_00cf1a18);

    DAT_00cf1a20 = command;
    DAT_00cf1a24 = index_mode;
    DAT_00cf1a1c = mode;
    std::uint8_t result = 1;
    if (command->vertex_count_26 == 0) {
        // Fresh batch: rewind both scratch cursors and aim the two
        // write-target pointers at the cursor variables (the exported x86
        // also refreshes the mirror slots even when indices are unused).
        DAT_00cf19f8 = DAT_00cf19f0;
        DAT_00cf1a00 = &DAT_00cf19f8;
        _DAT_00cf1a04 = DAT_00cf19f0;
        if ((index_mode & 1) == 0) {
            DAT_00cf19fc = DAT_00cf19f4;
            DAT_00cf1a08 = reinterpret_cast<std::uint32_t*>(&DAT_00cf19fc);
            _DAT_00cf1a0c = DAT_00cf19f4;
            command->word_1c = 0;
        }
    } else {
        // NOT YET INTEGRATED: 0x00650f70 (lock_scratch_buffers) -- the
        // scratch vertex/index pool Lock path (its 0x00651d60 /
        // 0x00651f80 acquirers create D3D9 buffers with vtable +0x68 /
        // +0x6c on DAT_00cf1944 and pool the entries' vtable +0x2c Lock).
        // Its full decompile is preserved in .ghidra-exports; until it is
        // integrated, a live batch with a nonzero vertex count keeps the
        // original's observable behavior short of buffer draining: the
        // exported x86's result byte feeding 0x00cf19de below.
        result = 1;
    }
    // The original stores the mode into DAT_00cf1a1c twice (before and
    // after the fresh/lock branch); the second store is what the rest of
    // the engine reads back.
    DAT_00cf1a1c = mode;
    DAT_00cf19de = result;
    return result;
}

// 0x006516f0 ----------------------------------------------------------------
// `source` points at `count` consecutive 0x40-byte records (16 dwords),
// packed exactly as the text emitter builds them (0x005e9820's four vertex
// records). Fields are appended in the mask order above from the source
// dword indices the original statement positions encode.
void write_batch_vertices(const void* source, std::uint32_t count) {
    void** write_slot = DAT_00cf1a00;
    if (write_slot == nullptr) {
        return;
    }
    std::uint8_t* out = static_cast<std::uint8_t*>(*write_slot);
    if (out == nullptr) {
        return; // the original skips the whole loop when *target == 0
    }
    const std::uint32_t* record = static_cast<const std::uint32_t*>(source);
    for (std::uint32_t i = 0; i < count; ++i) {
        std::memcpy(out, record, 0xc);
        out += 0xc;
        if ((DAT_00cf1a10 & 0x1000) != 0) {
            std::memcpy(out, record + 7, 0xc);
            out += 0xc;
        }
        if ((DAT_00cf1a10 & 0x10000) != 0) {
            std::memcpy(out, record + 10, 0xc);
            out += 0xc;
        }
        if ((DAT_00cf1a10 & 0x8000) != 0) {
            std::memcpy(out, record + 13, 0xc);
            out += 0xc;
        }
        if ((DAT_00cf1a10 & 0x8) != 0) {
            std::memcpy(out, record + 6, 0x4);
            out += 0x4;
        }
        if ((DAT_00cf1a10 & 0x100) != 0) {
            std::memcpy(out, record + 4, 0x8);
            out += 0x8;
        }
        if ((DAT_00cf1a10 & 0x10) != 0) {
            std::memcpy(out, record + 4, 0x4);
            out += 0x4;
        }
        record += 0x10;
    }
    *write_slot =
        static_cast<std::uint8_t*>(*write_slot) + count * DAT_00cf1a18;
}

} // namespace hp6::render
