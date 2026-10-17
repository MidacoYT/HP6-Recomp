#pragma once

// Archive container header access, integrated from the Ghidra exports of
// HP6 (x86). ReAgent was unavailable for this batch (LLM usage limit), so the
// integrations follow the exported decompilations directly. Equivalence with
// the binary remains unverified.
//
// Literals evidenced by the assembly: 0x42494746 ("BIGF"), 0x424947xx
// ("BIG" + any byte), 0x536870xx ("Sh" + any two bytes), and the 0xC0FB
// signed 16-bit marker. No owner-class or business field names are implied.

#include <cstdint>

namespace hp6::archive {

// 006dc532. Reads a <=4-byte big-endian field (evidenced by the "BIGF"
// literal 0x42494746 read from the bytes 'B','I','G','F' and by the P-code
// PIECE inputs). Sizes outside 1..4 return 0.
std::uint32_t read_partial_uint(const std::uint8_t* bytes, int count);

// 006dc594. Accumulates `count` bytes into a 64-bit big-endian register pair
// (EDX:EAX in the original); `bytes` is the raw byte storage address.
std::uint64_t pack_bytes_be(const std::uint8_t* bytes, int count);

// 006dc5f9. Container marker classification: 0 unknown, 1 = 0xC0FB short
// marker, 2 = full "BIGF" or "Sh??" prefix, 3 = "BIG" high-24 prefix.
std::uint32_t classify_container(const std::uint8_t* bytes);

// 006dc664. Header payload offset: type 1 -> 4 + 16-bit field at +2,
// types 2 and 3 -> 32-bit read at +0x0c. Type 0 returns 0.
std::uint32_t header_payload_offset(const std::uint8_t* bytes);

// 006dc6a3. Scans the four bytes preceding the header end (-8) for
// `[A-Za-z][0-9][0-9][0-9]`; yields 8 on success, otherwise 0. The ASCII
// digits are combined exactly as in the decompilation (signed char math,
// 0x14d0 bias) and the 32-bit read at the tag start goes to `tag_value`
// when non-null.
std::uint32_t extract_numeric_tag(const std::uint8_t* bytes,
                                  std::int32_t* digits_value,
                                  std::uint32_t* tag_value);

} // namespace hp6::archive
