// hp6::presentation::TextLayout — reconstruction of the text/layout side of
// the front-end render chain: everything between a filled render-text
// descriptor and the queued render command, i.e. the functions the two
// vtable-indirect descriptor producers (0x004228c0, the 64-widget walker on
// vtable 0x839380, and PluginPresentationBitmap_BuildRenderQuad on
// 0x827be0, reconstructed call-forward at 0x0056baa0) feed once per frame.
//
// Evidence trail (every function verified against the exported decompile
// AND raw x86; the decompile-vs-assembly pitfalls of this call site — stack
// shifts, adjacent locals, an ADD encoded as an x86 dword add that Ghidra
// showed as `+0x10` — are all noted inline):
//   0x005e92d0  layout_text_descriptor: gate checks, stack RunBuffer over a
//               0x400-byte scratch, SEH (__finally-equivalent) release,
//               then clip + submit.
//   0x005e93e0  format_text_runs: splits the wide string into runs. Plain
//               (flag2 clear) mode emits ONE run from the descriptor's
//               +0x24..+0x30 floats and +0x34..+0x3a u16s. Text (flag2 set)
//               mode walks the wide string: `\[` opens a tag (body measured
//               by 0x005e9640, resolved by 0x005e1e70 against the registry
//               at DAT_00fcaec0+0x14); runs tag-carrying and plain-glyph
//               runs in string order; a failed tag body/lookup emits
//               nothing and the loop resumes after the two-byte opener.
//   0x005e9640  measure_tag_body_length (thiscall on the descriptor).
//               Terminates the body at the `\]` sequence; every other char
//               counts. NUL or end without `]` returns 0.
//   0x005e1e70  lookup_tag_entry: converts the wide body to a narrow 64-byte
//               stack frame via wcstombs, applies the registry's optional
//               name transform, then walks the circular node list from
//               registry+0 (sentinel = registry itself), matching nodes
//               whose name starts with the body and continues with ':'.
//   0x005e9680  clip_runs_to_region: per run, gate byte = 1 when the run is
//               strictly non-overlapping with the descriptor's region, else
//               each touching boundary is clamped with the u16 UV-pair
//               lerp formulas below (the two clamps share the u/v pair and
//               the y_B clamp's divisor sees the y_B result of the y_A
//               clamp first — order preserved).
//   0x005e9820  submit_text_runs: per visible run, provisions a 0x30-byte
//               BatchCommand from the DAT_00cf18e4 bump pool (overflow
//               reproduces the original's deliberate null-store trap),
//               begins the vertex batch (format 0x12, mode 1, index mode 0
//               — GeometryBatcher), writes the four 0x40-byte vertex
//               records, the six index u16s from the DAT_8928d8 pattern
//               table (a flat {0,1,2, 1,3,2} quad), multiplies the
//               descriptor matrix with the caller context's +0x40 matrix
//               (multiply_matrix4x4), 0x10-aligns the product into the
//               DAT_0096b48c bump block, and queues the 5-dword packet
//               {aligned matrix, 0x8028, command, 0, descriptor+8} via
//               submit_render_command with the bucket chosen per descriptor
//               flag / tag entry.
//   0x00ffa950  resolve_run_font_bucket's body (0x005e1f30 is a pure
//               jump-thunk to it): entries sit at tag+0x10 with a 12-byte
//               stride {name, match_word, match_dword}, count at tag+0x90,
//               u16 results at tag+0x7c, fallback word DAT_00b1089c.
//   0x010108d0  insert_run (MSVC std::vector<LayoutRun,32-byte elem>
//               insert), growth moves one run at a time through
//   0x006295e0  move_run_block (forward 0x20-stride memmove; dest rides in
//               EAX — Ghidra's formal param_1/ECX is unused plumbing), and
//               reserves/frees with
//   0x005e27e0  allocate_eastl_block and
//   0x005e2860  release_eastl_block (byte-identical to the already
//               integrated 0x005dc710 release_through_service, minus its
//               null guard; our chain only calls it non-null, so it aliases
//               that reconstruction).
//
// The text path now calls 0x006512f0 before matrix submission. Its D3D9
// pool and Lock path is reconstructed in render/GeometryBatchFlush.cpp.
#pragma once

#include <cstddef>
#include <cstdint>

#include "platform/HandleRegistry.h" // release_through_service (0x005e2860's alias)

namespace hp6::presentation {

// ---------------------------------------------------------------------------
// Original record layouts.
// ---------------------------------------------------------------------------

// A staged layout run, 0x20 bytes ("vector element" of the two producer-owned
// templates and of the 0x400-byte stack scratch this chain builds over).
struct LayoutRun {
    // +0x00: pointer to the resolved tag node's name field (the value
    // lookup_tag_entry returns), or 0 in plain-glyph runs.
    std::uint32_t tag;
    // +0x04: only the low byte is meaningful: clip_runs_to_region writes
    // 0 (candidate) or 1 (fully outside the region -> submit_text_runs
    // skips the run). Bytes 4..7 are never written anywhere in this chain
    // (005e93e0 leaves them as the arena's previous content; zeroed here so
    // the reconstruction is deterministic — documented divergence).
    std::uint32_t gate;
    // +0x08..+0x14: the run quad, copied from the descriptor's corner
    // floats (+0x24..+0x30) by every run-building path.
    float x_left;   // +0x08 (consumed by vertex columns 0/2)
    float x_right;  // +0x0c (vertex columns 1/3)
    float y_a;      // +0x10 (vertex rows 0/1)
    float y_b;      // +0x14 (vertex rows 2/3)
    // +0x18..+0x1e: the four u16s. The x pair spans onto the x clamps, the
    // y pair onto the y clamps: the v_b entry is the one submit_text_runs
    // hands to the y_b vertex rows, v_a to the y_a rows. (No field in the
    // binary names these roles; the pairing below is exactly what the
    // vertex records encode.)
    std::uint16_t u_left;   // +0x18 (forms the x-axis span with u_right)
    std::uint16_t u_right;  // +0x1a
    std::uint16_t v_b;      // +0x1c (forms the y-axis span with v_a)
    std::uint16_t v_a;      // +0x1e
};
static_assert(sizeof(LayoutRun) == 0x20, "runs are the original 0x20 bytes");

// The growing run list 0x005e92d0 builds on the stack and forwards to
// 0x005e93e0/0x005e9680/0x005e9820. Field roles according to 010108d0:
// begin / cursor / cap_end are the classic triple; +0x0c is never written by
// this chain (the original copies an unnamed, uninitialized stack slot into
// it — Ghidra drops that dead store; there is no consumer anywhere in the
// chain); +0x10 (protected_begin) shields the stack scratch from insert_run's
// block free.
struct RunBuffer {
    LayoutRun* begin;            // +0x00
    LayoutRun* cursor;           // +0x04
    LayoutRun* cap_end;          // +0x08
    void* reserved_0c;           // +0x0c (stale in the original, unread)
    LayoutRun* protected_begin;  // +0x10
};

// The 0x54-byte descriptor both producers lay on their stack. Every field
// below is consumed by an integrated function in this file; the reserved
// dword at +0x50 is written by the producers but unread in this chain.
struct RenderTextDescriptor {
    std::uint32_t flags;        // +0x00: bit 2 (0x4) selects text/tag mode
    const wchar_t* text;        // +0x04: read only when flag 2 is set
    std::uint32_t value_08;     // +0x08: copied into the queued packet's 5th dword
    float rect_x_left;          // +0x0c: region rect; +0x0c <= +0x10 gates
    float rect_x_right;         // +0x10
    float rect_y_a;             // +0x14: pairs with run.y_a (+0x10); gates with +0x18
    float rect_y_b;             // +0x18
    const float* matrix;        // +0x1c: 16-float matrix (nonzero gates); lhs
    float value_20;             // +0x20: producer's DAT_007db234 constant
    // corner floats the runs are seeded with (+0x24..+0x30)
    float x_left;               // +0x24
    float x_right;              // +0x28
    float y_a;                  // +0x2c
    float y_b;                  // +0x30
    // source u16 quadruple (+0x34..+0x3a)
    std::uint16_t u_left;       // +0x34
    std::uint16_t u_right;      // +0x36
    std::uint16_t v_b;          // +0x38
    std::uint16_t v_a;          // +0x3a
    std::uint32_t value_3c;    // +0x3c: bitmap producer's this+0x30
    std::uint16_t default_bucket;  // +0x40: the queued bucket when flag 2 clear
    const char* match_name;        // +0x44: matcher name (the x86 reads bytes)
    std::uint32_t match_data_48;   // +0x48: passed low-word-first to the matcher
    std::uint32_t match_dword;     // +0x4c: matcher's third field
    std::uint32_t reserved_50;     // +0x50: producer-written, unread here
};
#if INTPTR_MAX == INT32_MAX
static_assert(offsetof(RenderTextDescriptor, x_left) == 0x24);
static_assert(offsetof(RenderTextDescriptor, default_bucket) == 0x40);
static_assert(offsetof(RenderTextDescriptor, match_name) == 0x44);
static_assert(sizeof(RenderTextDescriptor) == 0x54);
#endif

// ---------------------------------------------------------------------------
// Public functions (one per original address).
// ---------------------------------------------------------------------------

// 0x005e92d0. `transform_context` is the producer's own object (its +0x40
// float[16] is multiply_matrix4x4's rhs); the original also threads it into
// RunBuffer.reserved_0c.
void layout_text_descriptor(void* transform_context,
                            const RenderTextDescriptor& descriptor);

// 0x005e93e0 (its formal ECX plumbing is unused; the reconstruction takes
// the two real operands).
void format_text_runs(RunBuffer& runs, const RenderTextDescriptor& descriptor);

// 0x005e9640 (thiscall on the descriptor). Returns the body char count, 0
// when the body is empty/unterminated.
std::int32_t measure_tag_body_length(const RenderTextDescriptor& descriptor,
                                     std::int32_t start);

// 0x005e1e70. `registry` is the base the descriptor producers already hold
// (DAT_00fcaec0's value + 0x14): +0x0 = first node (sentinel = registry
// itself), +0xc = optional char*(char*) name transform. Returns a
// non-null pointer whose identity (registry node + 8) only matters as an
// opaque tag handle.
void* lookup_tag_entry(void* registry, const wchar_t* body,
                       std::size_t body_length);

// 0x005e9680.
void clip_runs_to_region(RunBuffer& runs,
                         const RenderTextDescriptor& descriptor);

// 0x005e9820.
void submit_text_runs(void* transform_context, RunBuffer& runs,
                      const RenderTextDescriptor& descriptor);

// 0x00ffa950 (0x005e1f30 is its jump-thunk; callers see it as stdcall).
// Tag layout: entries at tag+0x10, 12-byte stride {name char*, match_word
// u16, match_dword u32}; count u32 at tag+0x90; per-entry result u16 at
// tag+0x7c + 2*i. Fallback: the word at DAT_00b1089c.
std::uint16_t resolve_run_font_bucket(std::uint32_t tag, const char* name,
                                      std::uint16_t match_word,
                                      std::uint32_t match_dword);

// 0x010108d0 (0x006292b0 is its jump-thunk). MSVC std::vector insert
// semantics for 0x20-byte runs, including the exact grow factor and the
// original's allocation-overflow corner (new_count 0 -> zero begin, writes
// then skipped block-by-block).
void insert_run(RunBuffer& runs, LayoutRun* position, const LayoutRun& value);

// 0x006295e0 (__fastcall: dest rides in EAX, the formal ECX is unused
// plumbing). Forward-copies whole 0x20-byte runs; null dest blocks are
// walked (not written) preserving the original's overflow behavior.
LayoutRun* move_run_block(LayoutRun* dest, const LayoutRun* src,
                          const LayoutRun* src_end);

// 0x005e27e0. Lazy-inits the engine service holder (HandleRegistry's
// service_holder) then allocates `size` bytes through *DAT_00fcdd78 slot 0
// with the {1, "eastl_overflow_error_allocator", 0} context frame.
void* allocate_eastl_block(std::uint32_t size);

// 0x005e2860. Aliases hp6::platform::release_through_service (the original
// variants differ only by that reconstruction's null guard, which 005e2860
// lacks — this chain's callers guarantee non-null).
inline void release_eastl_block(void* block) {
    hp6::platform::release_through_service(block);
}

} // namespace hp6::presentation
