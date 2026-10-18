#pragma once
#include <cstddef>
#include <cstdint>

namespace hp6::runtime {

// 0x00422e20 (thiscall). Stores `size` bytes of `data` either in the
// 0x800-byte inline buffer at self+0x10 (size < 0x800) or in a shared
// buffer reached through DAT_00fcade0+0x18 (0x800 <= size <= 0x23ff; sizes
// above 0x23ff, and a null shared buffer, are silently dropped). Remembers
// the destination at self+0x810 and sets the byte flag at self+0x8 to 1.
// ReAgent's reviewer could not independently confirm the branch polarity
// of the two signed comparisons from the supplied P-code/CFG alone; the
// exported x86 (JGE 0x00422e34 on `size>=0x800`, JGE 0x00422e62 on
// `size>=0x2400`) matches the polarity used here exactly.
void store_payload(void* self, const void* data, std::size_t size);

// 0x0063b850 (fastcall; the first argument is unused by the callee, but
// callers pass their own `this`/record pointer through it). Bump-allocates
// `size` bytes (rounded up to 4) from the arena pointed to by
// DAT_00cf18e4 ([0]=base, [1]=capacity, [2]=current). The current field is
// always advanced by the aligned size before the bound is checked; once
// the new position reaches or passes base+capacity, the exported x86
// writes to address 0 with no recovery path (a fatal out-of-memory trap),
// reproduced here exactly. Returns the pre-advance current value.
std::uint32_t bump_allocate(std::uint32_t unused_param_1, std::uint32_t size);

// 0x006e77cb (cdecl; the third argument is unused). Copies `count` 32-bit
// words from the cursor pointer stored at frame+0x1c into `destination`,
// then advances that cursor by the copied byte count. Returns `count`.
std::int32_t advance_cursor_copy(std::uint8_t* frame, std::int32_t count,
                                 std::uint32_t unused_param_3,
                                 void* destination);

// 0x01003620 (cdecl). Copies DAT_00cf19a0 * 8 bytes from the fixed buffer
// _Src_00c754d0 into `destination` and returns DAT_00cf19a0 (a record
// count); neither global's true size is established here.
std::int32_t copy_fixed_records(void* destination);

// 0x01003ca0 (cdecl). DAT_00cf1a08 points to a cursor variable; when that
// cursor is non-null, appends `count * 2` bytes from `source` at the
// cursor and advances it by the same amount. A no-op when the cursor is
// null. ReAgent's reviewer judged the double dereference an added level
// over the P-code; the exported x86 (MOV EAX,[DAT_00cf1a08]; CMP
// [EAX],0) performs exactly the two loads reproduced here.
void append_to_cursor(const void* source, std::int32_t count);

// 0x01002890 (thiscall; `self` is only forwarded, unused, into
// bump_allocate). Bump-allocates a 0x30-byte record and copies `source`'s
// first 12 words into it verbatim. When `copy_a` is set, stores
// DAT_00866368 at the record's word 4 and copies
// source[9](byte) * source_u16(source+0x26) bytes from *source[10] into
// the fixed buffer _Dst_00bb0a70. When `copy_b` is set, stores the same
// DAT_00866368 at word 5 and copies source_u16(source+0x22) * 2 bytes from
// *source[11] into _Dst_0096b490. Only the low byte of the exported x86
// return value is defined (1 on completion); the upper bytes are
// otherwise-unconstrained leftovers in EAX, so this returns a plain 1.
std::uint32_t init_payload_record(void* self, const std::uint32_t* source,
                                  char copy_a, char copy_b);

} // namespace hp6::runtime
