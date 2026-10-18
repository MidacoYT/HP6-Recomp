#pragma once
#include <cstdint>

namespace hp6::runtime {

// 0x006b4678 (fastcall). Zeroes 4 words (16 bytes) at `block`.
void clear_quad_word(std::uint32_t* block);

// 0x006b6808 (fastcall). Zeroes a 0x61-byte record: 0x40 bytes (name
// buffer), 4 words at +0x40..+0x4c, a quad word at +0x50..+0x5c
// (clear_quad_word), and the byte at +0x60.
void clear_full_record(std::uint32_t* block);

// 0x006b6833 (thiscall). Zeroes the record above (clear_full_record),
// then copies up to 0x40 bytes of `name` into it, stores four words at
// +0x40..+0x4c, optionally copies a 4-word id from `optional_id` into
// +0x50..+0x5c when non-null, and clears the byte at +0x60 again
// (redundant with the prior zeroing, matching the exported x86 exactly).
void construct_full_record(void* record, const char* name, std::uint32_t field_0x40,
                           std::uint32_t field_0x44, std::uint32_t field_0x48,
                           std::uint32_t field_0x4c, const std::uint32_t* optional_id);

// 0x006b687f (fastcall). Zeroes a 0x50-byte record: 0x40 bytes (name
// buffer) and a quad word at +0x40..+0x4c (clear_quad_word).
void clear_named_record(std::uint32_t* block);

// 0x006b6894 (thiscall). Zeroes the record above (clear_named_record),
// then copies up to 0x40 bytes of `name` into it, and optionally copies a
// 4-word id from `optional_id` into +0x40..+0x4c when non-null.
void construct_named_record(void* record, const char* name,
                            const std::uint32_t* optional_id);

// 0x006b4c17 (thiscall). Clears a 32-byte name buffer at this+0xb4, then
// copies up to 0x1f bytes of `name` into it and sets five fixed fields:
// this+0x4dc=-1, this+0x4e4=0, this+0x4d8=4, this+0x6c=4, this+0x70=9.
// The record this belongs to, and these fields' meaning, are unresolved.
void initialize_object_name_header(void* object, const char* name);

} // namespace hp6::runtime
