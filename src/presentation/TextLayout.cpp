#include "presentation/TextLayout.h"

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <cwchar>

#include "math/Transform.h"
#include "render/GeometryBatcher.h"
#include "render/SubmitRenderCommand.h"

// Game address-space symbols shared with the other reconstruction files.
// Types mirror src/runtime/ScratchBuffers.cpp, src/render/SubmitRenderCommand.cpp,
// src/render/GeometryBatcher.cpp and src/TaskSystem.cpp (one extern per
// symbol, one set of types across translation units).
extern std::int32_t* DAT_00cf18e4;             // bump arena [0]=base [1]=capacity [2]=current
extern std::uint32_t* DAT_00cf1a08;            // -> scratch index cursor variable
extern volatile std::uint32_t _DAT_00000000;   // deliberate null-store trap target
extern void* DAT_0096b48c;                     // shared bump write-pointer (matrices/commands)
extern void* DAT_00fcdd78;                     // allocator service holder slot 1
extern void* DAT_00fcaec0;                     // pointer-valued global; its value+0x14 is the
                                               // tag registry (+0x0 first node, +0xc transform)
extern std::uint16_t DAT_00d714b8;             // default submit bucket (text mode, no tag)
extern std::uint16_t DAT_00b1089c;             // tag matcher fallback bucket word
extern std::uint32_t DAT_00728004;             // plain-glyph corner constant
extern std::uint32_t DAT_0082ae34;             // plain-glyph corner constant
extern std::uint32_t DAT_008928d8;             // index pattern: this dword
                                               // carries the first word pair
extern std::uint32_t DAT_008928dc;             // ... 0x008928dc carries the
                                               // second pair (slot 1)

namespace hp6::presentation {

namespace {

// 0x005e92d0 builds the RunBuffer over exactly 0x400 bytes of stack scratch
// (cap_end == scratch + 0x400 == 32 runs).
constexpr std::size_t run_scratch_runs = 0x400u / sizeof(LayoutRun);
static_assert(run_scratch_runs == 32, "original stack scratch holds 32 runs");

// x86: (short)(int)(t * (float)(int)((uint)span_hi - (uint)span_base)
//                    + (float)span_base)
// The u16 span is computed as a 32-bit unsigned difference; the (short)
// stores truncate wider float results toward zero, as cvttss2si does.
std::int16_t lerp_u16(float position_ratio, std::uint16_t span_hi,
                      std::uint16_t span_base) {
    const float span = static_cast<float>(static_cast<std::int32_t>(
        static_cast<std::uint32_t>(span_hi) -
        static_cast<std::uint32_t>(span_base)));
    const float value =
        position_ratio * span + static_cast<float>(span_base);
    return static_cast<std::int16_t>(static_cast<std::int32_t>(value));
}

std::uint32_t bits_of(float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    return bits;
}

// run source records for 0x006516f0: 0x40 bytes (16 dwords) apiece, only
// dwords 0, 1, 2 and the u16 pair packed in dword 4 are consumed by the
// format 0x12 mask; the original leaves the rest as stale stack bytes and
// this reconstruction zeroes them (deterministic divergence, as in
// submit_text_runs' packet tail).
struct VertexSourceRecord {
    std::uint32_t dword[16];
};
static_assert(sizeof(VertexSourceRecord) == 0x40, "original record is 0x40");

// The repeated append sequence of 0x005e93e0 (three inline copies there):
// fit-on-one-line = cursor as-is; else reallocate through 0x010108d0.
void append_staged_run(RunBuffer& runs, const LayoutRun& staged) {
    LayoutRun* cursor = runs.cursor;
    if (cursor < runs.cap_end) {           // == puVar3 < param_1[2]
        runs.cursor = cursor + 1;          // puVar3 + 8 dwords
        if (cursor != nullptr) {
            *cursor = staged;              // MOVSD.REP 8 dwords
        }
    } else {
        insert_run(runs, cursor, staged);  // thunk_FUN_010108d0
    }
}

} // namespace

// 0x005e92d0 -----------------------------------------------------------------
void layout_text_descriptor(void* transform_context,
                            const RenderTextDescriptor& descriptor) {
    // Gate order per the exported x86: flag-2 text pointer, matrix, then
    // the two COMISS rect pairs (subceeding == bail; equality passes).
    if ((((descriptor.flags & 4) == 0) || (descriptor.text != nullptr)) &&
        descriptor.matrix != nullptr && descriptor.rect_x_left <= descriptor.rect_x_right &&
        descriptor.rect_y_a <= descriptor.rect_y_b) {
        LayoutRun scratch[run_scratch_runs];
        RunBuffer runs;
        runs.begin = scratch;
        runs.cursor = scratch;
        runs.cap_end = scratch + run_scratch_runs;
        // The original copies an unnamed stack slot here (dead store, no
        // consumer); left null for determinism — see RunBuffer's contract.
        runs.reserved_0c = nullptr;
        runs.protected_begin = scratch;

        // The original wraps the body in an SEH scope whose __finally
        // funclet releases the grown block when begin is non-null and not
        // the stack scratch itself. The guard below runs on normal exit
        // and on unwind alike, with the same two-condition test.
        struct ReleaseOnExit {
            RunBuffer& runs;
            ~ReleaseOnExit() {
                if (runs.begin != nullptr && runs.begin != runs.protected_begin) {
                    release_eastl_block(runs.begin);
                }
            }
        } release_on_exit{runs};

        format_text_runs(runs, descriptor);
        if ((descriptor.flags & 4) == 0 ||
            ((reinterpret_cast<std::uintptr_t>(runs.cursor) -
              reinterpret_cast<std::uintptr_t>(runs.begin)) &
             static_cast<std::uintptr_t>(0xffffffe0u)) != 0) {
            clip_runs_to_region(runs, descriptor);
            submit_text_runs(transform_context, runs, descriptor);
        }
    }
}

// 0x005e93e0 -----------------------------------------------------------------
void format_text_runs(RunBuffer& runs, const RenderTextDescriptor& descriptor) {
    if ((descriptor.flags & 4) == 0) {
        // Plain mode: one run cloned from the descriptor's corner floats
        // and u16 quadruple, no tag.
        LayoutRun staged = {};
        staged.tag = 0;
        staged.x_left = descriptor.x_left;
        staged.x_right = descriptor.x_right;
        staged.y_a = descriptor.y_a;
        staged.y_b = descriptor.y_b;
        staged.u_left = descriptor.u_left;
        staged.u_right = descriptor.u_right;
        staged.v_b = descriptor.v_b;
        staged.v_a = descriptor.v_a;
        append_staged_run(runs, staged);
        return;
    }

    // Text/tag mode: walk the wide string. PSVar6 is re-read from the
    // descriptor every iteration (the original does the same), and each
    // plain character emits one run over the shared corner constants.
    std::int32_t index = 0;
    if (descriptor.text[0] != 0) {
        do {
            std::int32_t next = index;
            if (descriptor.text[index] == L'\\') {
                next = index + 1;
                if (descriptor.text[next] == L'[') {
                    const std::int32_t body_length =
                        measure_tag_body_length(descriptor, index + 2);
                    if (body_length != 0) {
                        void* tag_node = lookup_tag_entry(
                            reinterpret_cast<std::uint8_t*>(DAT_00fcaec0) + 0x14,
                            descriptor.text + index + 2,
                            static_cast<std::size_t>(body_length));
                        if (tag_node != nullptr) {
                            LayoutRun staged = {};
                            staged.tag = static_cast<std::uint32_t>(
                                reinterpret_cast<std::uintptr_t>(tag_node));
                            staged.x_left = descriptor.x_left;
                            staged.x_right = descriptor.x_right;
                            staged.y_a = descriptor.y_a;
                            staged.y_b = descriptor.y_b;
                            staged.u_left = descriptor.u_left;
                            staged.u_right = descriptor.u_right;
                            staged.v_b = descriptor.v_b;
                            staged.v_a = descriptor.v_a;
                            append_staged_run(runs, staged);
                            // Resume at the ']' (body_len + 2 + (index + 1)).
                            next = body_length + 2 + next;
                        }
                    }
                    // Body measured but unresolved (or empty): the
                    // original skips the `\[` pair without emitting a run
                    // and resumes with next == index + 1.
                }
            } else {
                // Plain glyph: the corners come out of DAT_00728004 /
                // DAT_0082ae34 as raw dword copies into the float fields,
                // and the u16 quadruple is the fixed 0/0x1000 flat pair.
                LayoutRun staged = {};
                staged.tag = 0;
                std::memcpy(&staged.x_left, &DAT_00728004, 4);
                std::memcpy(&staged.x_right, &DAT_0082ae34, 4);
                std::memcpy(&staged.y_a, &DAT_00728004, 4);
                std::memcpy(&staged.y_b, &DAT_0082ae34, 4);
                staged.u_left = 0;
                staged.u_right = 0x1000;
                staged.v_b = 0;
                staged.v_a = 0x1000;
                next = index;
                append_staged_run(runs, staged);
            }
            const wchar_t* text = descriptor.text; // re-read per iteration
            const wchar_t peeked = text[next + 1];
            index = next + 1;
            if (peeked == 0) {
                break;
            }
        } while (true);
    }
}

// 0x005e9640 (thiscall) --------------------------------------------------------
std::int32_t measure_tag_body_length(const RenderTextDescriptor& descriptor,
                                     std::int32_t start) {
    const wchar_t* p = descriptor.text + start;
    std::int32_t count = 0;
    for (;;) {
        if (*p == 0) {
            return 0;
        }
        if (*p == L'\\') {
            ++p;                    // now on the escape's second character
            if (*p == L']') {
                return count;       // `\]` terminates the body
            }
            p += 1;                 // the escape follower is consumed uncounted
            continue;
        }
        count += 1;
        p += 1;
    }
}

// 0x005e1e70 -------------------------------------------------------------------
void* lookup_tag_entry(void* registry, const wchar_t* body,
                       std::size_t body_length) {
    // 0x40-byte narrow stack frame; the original relies on the
    // pre-zeroed tail as the terminator. wcstombs with a counted body
    // stores no NUL itself, so the frame's zeroed byte serves.
    // DIVERGENCE: the original passes body_length unclamped and therefore
    // overruns the frame for bodies of 64+ characters; this reconstruction
    // clamps the conversion to 63 so the terminator survives (input
    // contract: bodies longer than 63 characters are unsupported, as they
    // are stack-unsafe in the original too).
    char frame[64];
    frame[0] = '\0';
    std::memset(frame + 1, 0, sizeof(frame) - 1);
    char* narrow = frame;
    const std::size_t convert_length =
        body_length < sizeof(frame) - 1 ? body_length : sizeof(frame) - 1;
    std::wcstombs(narrow, body, convert_length);
    narrow[sizeof(frame) - 1] = '\0';

    // Optional registry-provided name transform (+0xc slot).
    auto* transform =
        *reinterpret_cast<char* (*const*)(char*)>(
            reinterpret_cast<std::uint8_t*>(registry) + 0xc);
    if (transform != nullptr) {
        narrow = transform(narrow);
    }
    const std::size_t narrow_length = std::strlen(narrow);

    // Circular walk from registry+0 with the registry itself as sentinel
    // foot; a hit must match the body then continue with ':'.
    void* node = *static_cast<void* const*>(registry);
    while (node != registry && node != nullptr) {
        const char* name = static_cast<const char*>(static_cast<void**>(node)[2]);
        if (name != nullptr && std::strncmp(name, narrow, narrow_length) == 0 &&
            name[narrow_length] == ':') {
            return static_cast<void**>(node) + 2;  // = node + 8 (the tag handle)
        }
        node = *static_cast<void* const*>(node);
    }
    return nullptr;
}

// 0x005e9680 -------------------------------------------------------------------
void clip_runs_to_region(RunBuffer& runs,
                         const RenderTextDescriptor& descriptor) {
    for (LayoutRun* run = runs.begin; run != runs.cursor; ++run) {
        auto* gate_byte = reinterpret_cast<std::uint8_t*>(&run->gate);
        *gate_byte = 0;
        const float run_x_left = run->x_left;
        const float rect_x_left = descriptor.rect_x_left;
        // Outer gate: the run is not strictly inside the region.
        const bool touches_boundary = run_x_left <= rect_x_left ||
                                       descriptor.rect_x_right <= run->x_right ||
                                       descriptor.rect_y_b <= run->y_b ||
                                       run->y_a <= descriptor.rect_y_a;
        if (!touches_boundary) {
            continue;
        }
        // Inner gate: strictly disjoint from the region -> suppressed.
        // (The x86 loads the four run fields for these conditions and the
        // clamp branches reuse those same register values; the snapshots
        // below preserve that.)
        const float run_x_right = run->x_right;
        const float run_y_bottom = run->y_b;
        const float run_y_top = run->y_a;
        if ((descriptor.rect_x_right <= run_x_left &&
             run_x_left != descriptor.rect_x_right) ||
            run_x_right < rect_x_left ||
            run_y_bottom < descriptor.rect_y_a ||
            (descriptor.rect_y_b <= run_y_top && run_y_top != descriptor.rect_y_b)) {
            *gate_byte = 1;
        } else {
            if (run_x_left < rect_x_left) {
                run->u_left =
                    static_cast<std::uint16_t>(lerp_u16(
                        (rect_x_left - run_x_left) / (run_x_right - run_x_left),
                        run->u_right, run->u_left));
                run->x_left = rect_x_left;
            }
            if (descriptor.rect_x_right <= run_x_right &&
                run_x_right != descriptor.rect_x_right) {
                // Reads the CURRENT x_left (branch 1 may have moved it).
                const float current_x_left = run->x_left;
                run->u_right =
                    static_cast<std::uint16_t>(lerp_u16(
                        (descriptor.rect_x_right - current_x_left) /
                            (run_x_right - current_x_left),
                        run->u_right, run->u_left));
                run->x_right = descriptor.rect_x_right;
            }
            if (descriptor.rect_y_b <= run_y_bottom &&
                run_y_bottom != descriptor.rect_y_b) {
                // v span/base are the (+0x1c, +0x1e) pair; the ratio uses
                // the y_top snapshot (run+0x10, untouched so far).
                run->v_b = static_cast<std::uint16_t>(lerp_u16(
                    (descriptor.rect_y_b - run_y_top) /
                        (run_y_bottom - run_y_top),
                    run->v_b, run->v_a));
                run->y_b = descriptor.rect_y_b;
            }
            if (run_y_top < descriptor.rect_y_a) {
                // Divisor re-reads y_b AFTER branch 3's clamp.
                const float current_y_bottom = run->y_b;
                run->v_a = static_cast<std::uint16_t>(lerp_u16(
                    (descriptor.rect_y_a - run_y_top) /
                        (current_y_bottom - run_y_top),
                    run->v_b, run->v_a));
                run->y_a = descriptor.rect_y_a;
            }
        }
    }
}

// 0x005e9820 -------------------------------------------------------------------
void submit_text_runs(void* transform_context, RunBuffer& runs,
                      const RenderTextDescriptor& descriptor) {
    const std::uint16_t* pattern_words =
        reinterpret_cast<const std::uint16_t*>(&DAT_008928d8);
    const std::uint16_t* pattern_words_dc =
        reinterpret_cast<const std::uint16_t*>(&DAT_008928dc);
    for (LayoutRun* run = runs.begin; run != runs.cursor; ++run) {
        if (reinterpret_cast<const std::uint8_t*>(&run->gate)[0] != 0) {
            continue; // suppressed by clip_runs_to_region
        }
        // 0x30-byte BatchCommand from the DAT_00cf18e4 bump pool. The
        // advance happens first; when the pool reaches or crosses its
        // limit (base + capacity) the original performs its deliberate
        // null-store trap, reproduced here with the shared trap target.
        std::int32_t* arena = DAT_00cf18e4;
        std::uint8_t* command =
            reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(static_cast<std::uint32_t>(arena[2])));
        arena[2] += 0x30;
        if (arena[1] + arena[0] <= arena[2]) {
            _DAT_00000000 = 0;
        }
        std::memset(command, 0, 0x30);
        *reinterpret_cast<std::uint32_t*>(command + 0x18) = 0x12;
        *reinterpret_cast<std::uint16_t*>(command + 0x26) = 0;
        *reinterpret_cast<std::uint16_t*>(command + 0x22) = 0;
        command[0x25] = 4;
        hp6::render::begin_vertex_batch(
            reinterpret_cast<hp6::render::BatchCommand*>(command), 1, 0);

        // Four 0x40-byte vertex source records: (x, y, 0, -, {u_lo, u_hi})
        // where the u16 pairs follow the vertex/row pairing of LayoutRun's
        // comment (y_a rows take v_a, y_b rows take v_b).
        VertexSourceRecord records[4] = {};
        records[0].dword[0] = bits_of(run->x_left);
        records[0].dword[1] = bits_of(run->y_a);
        records[0].dword[2] = 0;
        records[0].dword[4] = static_cast<std::uint32_t>(run->u_left) |
                              (static_cast<std::uint32_t>(run->v_a) << 16);
        records[1].dword[0] = bits_of(run->x_right);
        records[1].dword[1] = bits_of(run->y_a);
        records[1].dword[2] = 0;
        records[1].dword[4] = static_cast<std::uint32_t>(run->u_right) |
                              (static_cast<std::uint32_t>(run->v_a) << 16);
        records[2].dword[0] = bits_of(run->x_left);
        records[2].dword[1] = bits_of(run->y_b);
        records[2].dword[2] = 0;
        records[2].dword[4] = static_cast<std::uint32_t>(run->u_left) |
                              (static_cast<std::uint32_t>(run->v_b) << 16);
        records[3].dword[0] = bits_of(run->x_right);
        records[3].dword[1] = bits_of(run->y_b);
        records[3].dword[2] = 0;
        records[3].dword[4] = static_cast<std::uint32_t>(run->u_right) |
                              (static_cast<std::uint32_t>(run->v_b) << 16);
        hp6::render::write_batch_vertices(records, 4);

        // Six index u16s: per pass i (0..1), a 4-byte-strided word pair
        // from the 0x008928d8 table selected by the inverted bit of i,
        // premultiplied by i. With the table's dwords {0x00010002,
        // 0x00020001} this is the flat {0,1,2, 1,3,2} quad.
        std::uint16_t* index_cursor =
            reinterpret_cast<std::uint16_t*>(static_cast<std::uintptr_t>(*DAT_00cf1a08));
        for (std::int32_t pass = 0; pass < 2; ++pass) {
            const std::uint32_t slot = static_cast<std::uint32_t>(
                ~static_cast<std::uint8_t>(pass) & 1);
            const std::uint16_t* pair =
                slot == 0 ? pattern_words : pattern_words_dc;
            *index_cursor++ = static_cast<std::uint16_t>(pass);
            *index_cursor++ = static_cast<std::uint16_t>(
                pair[0] + static_cast<std::uint16_t>(pass));
            *index_cursor++ = static_cast<std::uint16_t>(
                pair[1] + static_cast<std::uint16_t>(pass));
        }
        *DAT_00cf1a08 = static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(index_cursor));
        *reinterpret_cast<std::uint16_t*>(command + 0x1e) += 2;

        command[0] = 0x50;
        command[1] = 0x48;
        *reinterpret_cast<std::uint32_t*>(command + 0x8) = 0x12;

        hp6::render::flush_vertex_batch();

        float product[16];
        hp6::math::multiply_matrix4x4(
            product, descriptor.matrix,
            reinterpret_cast<const float*>(
                static_cast<const std::uint8_t*>(transform_context) + 0x40));
        // 0x10-aligned matrix slot in the DAT_0096b48c bump block (the raw
        // x86 aligns first, then advances the pointer by the full 0x40).
        std::uint8_t* aligned = reinterpret_cast<std::uint8_t*>(
            (reinterpret_cast<std::uintptr_t>(DAT_0096b48c) + 0xf) &
            ~static_cast<std::uintptr_t>(0xf));
        DAT_0096b48c = aligned;
        std::memcpy(aligned, product, 0x40);
        DAT_0096b48c = aligned + 0x40;

        // 5-dword submission packet. The original's record is 0x14 bytes of
        // defined data; submit_render_command's 0x24-byte copy path reads
        // 0x10 further stale stack words, which this reconstruction zeroes
        // (deterministic divergence, like the vertex records' pad dwords).
        std::uint32_t packet[9] = {};
        packet[0] = static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(aligned));
        packet[1] = 0x8028;
        packet[2] = static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(command));
        packet[3] = 0;
        packet[4] = descriptor.value_08;

        std::int16_t bucket;
        if ((descriptor.flags & 4) == 0) {
            bucket = static_cast<std::int16_t>(descriptor.default_bucket);
        } else {
            bucket = static_cast<std::int16_t>(DAT_00d714b8);
            if (run->tag != 0) {
                bucket = static_cast<std::int16_t>(resolve_run_font_bucket(
                    run->tag, descriptor.match_name,
                    static_cast<std::uint16_t>(descriptor.match_data_48),
                    descriptor.match_dword));
            }
        }
        hp6::render::submit_render_command(
            bucket, reinterpret_cast<std::int32_t*>(packet));
    }
}

// 0x00ffa950 (0x005e1f30 is its jump-thunk) --------------------------------------
std::uint16_t resolve_run_font_bucket(std::uint32_t tag, const char* name,
                                      std::uint16_t match_word,
                                      std::uint32_t match_dword) {
    const std::uint32_t count =
        *reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(tag) + 0x90);
    const std::uint32_t* entry =
        reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(tag) + 0x18);
    for (std::uint32_t i = 0; i < count; ++i, entry += 3) {
        const char* entry_name =
            reinterpret_cast<const char*>(entry[-2]); // tag+0x10+12i
        // The decompile's manual byte-by-byte compare reduces to an
        // equality test against the NUL-terminated match name.
        if (std::strcmp(entry_name, name) == 0 &&
            static_cast<std::uint16_t>(entry[-1]) == match_word &&
            entry[0] == match_dword) {
            return *reinterpret_cast<const std::uint16_t*>(
                static_cast<std::uintptr_t>(tag) + 0x7c + i * 2);
        }
    }
    return DAT_00b1089c;
}

// 0x010108d0 (0x006292b0 is its jump-thunk) ---------------------------------------
void insert_run(RunBuffer& runs, LayoutRun* position, const LayoutRun& value) {
    if (runs.cursor != runs.cap_end) {
        const LayoutRun* source_value = &value;
        // Pointer-identity adjustment for a value that lives inside the
        // move range (the callers stage on the stack, so it never fires).
        if (position <= source_value && source_value < runs.cursor) {
            source_value = source_value + 1;
        }
        if (runs.cursor != nullptr) {
            *runs.cursor = *(runs.cursor - 1); // shift the last run right
        }
        LayoutRun* destination = runs.cursor;
        LayoutRun* source = runs.cursor - 1;
        while (source != position) {
            --source;
            --destination;
            *destination = *source;
        }
        *position = *source_value;
        runs.cursor += 1;
        return;
    }

    // Grow path: classic MSVC std::vector doubling, allocation through the
    // eastl allocator pair.
    const std::size_t count =
        static_cast<std::size_t>(runs.cursor - runs.begin);
    const std::size_t new_count = (count == 0) ? 1 : count * 2;
    if (new_count == 0) {
        // Allocation-count overflow corner: the original walks its moves
        // with a null destination (writes skipped) and then stores the
        // staged value through a near-null derived pointer — fatal there,
        // reproduced as the shared deterministic null-store trap here (the
        // original's unwritten destination-block walk is not observable).
        _DAT_00000000 = 0;
        return;
    }
    LayoutRun* new_begin = static_cast<LayoutRun*>(
        allocate_eastl_block(static_cast<std::uint32_t>(new_count << 5)));
    // Move [begin, position) -> new_begin, insert the value, then move
    // [position, cursor) right behind it.
    LayoutRun* placed = move_run_block(new_begin, runs.begin, position);
    *placed = value;
    LayoutRun* new_cursor =
        move_run_block(placed + 1, position, runs.cursor);
    if (runs.begin != nullptr && runs.begin != runs.protected_begin) {
        release_eastl_block(runs.begin);
    }
    runs.cap_end = new_begin + new_count;
    runs.begin = new_begin;
    runs.cursor = new_cursor;
}

// 0x006295e0 -----------------------------------------------------------------------
LayoutRun* move_run_block(LayoutRun* dest, const LayoutRun* src,
                          const LayoutRun* src_end) {
    LayoutRun* destination = dest;
    const LayoutRun* source = src;
    while (source != src_end) {
        if (destination != nullptr) {
            *destination = *source; // 8 dwords per block
        }
        ++destination;
        ++source;
    }
    return destination;
}

// 0x005e27e0 -------------------------------------------------------------------------
void* allocate_eastl_block(std::uint32_t size) {
    // Shared lazy-init (identical to the release path's).
    hp6::platform::service_holder();
    // {overflow-level 1, context string, null} context frame passed as the
    // second stack argument of 0x00fcdd78-slot-0's allocator.
    struct AllocatorContext {
        std::int32_t level = 1;
        const char* name = "eastl_overflow_error_allocator";
        void* previous = nullptr;
    } context;
    // *DAT_00fcdd78: the allocator function pointer lives AT the address
    // DAT_00fcdd78 holds (slot 0 of the 0x0083e56c table).
    auto* allocate =
        *reinterpret_cast<void* (**)(std::uint32_t, void*)>(DAT_00fcdd78);
    return allocate(size, &context);
}

} // namespace hp6::presentation
