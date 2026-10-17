#pragma once

// Archive entry lookup, integrated from the Ghidra exports of HP6 (x86).
// ReAgent was unavailable for this batch (LLM usage limit); integrations
// follow the exported decompilations and assembly directly, including the
// decompiler's double-width name pointer and the unallocated second-string
// register of 006dc5c9. Equivalence remains unverified.

#include <cstdint>

namespace hp6::archive {

// 006dc5c9. Case-insensitive byte comparison through CRT tolower; returns
// the signed difference of the first mismatching lowered bytes (0 when both
// match up to a terminator). The exported assembly leaves the second string
// in EAX and the first in ECX; the plain two-string form below keeps the
// observable behavior, and the register ABI is documented for parity only.
std::int32_t case_insensitive_compare(const char* left, const char* right);

// 006dc751. Walks entries stored after the container header. `requested`
// selects compare-by-name mode (its caller passes the match string; nullptr
// selects walk-by-index mode with `index`). Beginning at the type-dependent
// cursor (0x10, or 6 for the 0xC0FB marker), every record is [width-byte
// big-endian field][name string]; the internal name pointer adds the width
// twice, exactly as in the exported assembly. `field_value` receives the
// 32-bit low part of the width-byte big-endian pack, `raw_value` the
// width-byte read stored right after it. Returns the internal name pointer,
// or nullptr after clearing whichever outputs are present.
const std::uint8_t* find_entry(const std::uint8_t* bytes,
                               const char* requested, int index,
                               std::int32_t* field_value,
                               std::uint32_t* raw_value);

// 006dc86f. Compare-by-name wrapper returning the field low 32 bits
// (0 when the name does not resolve or the requested pointer is null).
std::int32_t entry_field_by_name(const std::uint8_t* bytes,
                                 const char* requested);

// 006dc962. Byte-for-byte twin of entry_field_by_name (distinct address).
std::int32_t entry_field_by_name_twin(const std::uint8_t* bytes,
                                      const char* requested);

// 006dc896. Index wrapper returning bytes + low 32-bit field of the selected
// entry, or nullptr.
const std::uint8_t* entry_pointer_by_index(const std::uint8_t* bytes,
                                           int index);

// 006dc920. Copies the internal name string of entry `index` to `output`
// and returns `output`; writes a single NUL and returns nullptr when absent.
char* copy_entry_string(const std::uint8_t* bytes, int index, char* output);

// 006dc9eb. Byte-for-byte twin of copy_entry_string (distinct binary address).
char* copy_entry_string_twin(const std::uint8_t* bytes, int index,
                             char* output);

// 006dc959. Full-signature pass-through to find_entry.
void entry_forward(const std::uint8_t* bytes, const char* requested,
                   int index, std::int32_t* field_value,
                   std::uint32_t* raw_value);

// 006dc733. The signed 16-bit digit sum produced by extract_numeric_tag
// (0 when no well-formed 8-byte tag block precedes the payload).
std::int32_t tag_digits_value(const std::uint8_t* bytes);

// 006dc8e4. Secondary 32-bit header field: 16-bit field at +4 for the
// 0xC0FB marker, 32-bit big-endian read at +8 for types 2 and 3, else 0.
std::uint32_t header_field_32(const std::uint8_t* bytes);

// 006dc8c2. bytes + field low 32 bits of the named entry, nullptr when the
// name does not resolve or the value is zero.
const std::uint8_t* entry_pointer_by_name(const std::uint8_t* bytes,
                                          const char* requested);

// 006dc989. Twin of entry_pointer_by_name (distinct address).
const std::uint8_t* entry_pointer_by_name_twin(const std::uint8_t* bytes,
                                               const char* requested);

// 006dc9ab. Splits a comma-separated name list and writes the resolved
// entry pointers (entry_pointer_by_name twin) into consecutive slots of
// `out_ptrs`. The comma separator is restored between iterations.
void resolve_entry_list(const std::uint8_t* bytes, char* names,
                        const std::uint8_t** out_ptrs);

} // namespace hp6::archive
