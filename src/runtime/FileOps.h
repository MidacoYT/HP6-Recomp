#pragma once

#include <cstdint>

namespace hp6::runtime {

// Reserved-callback file operators (the exported batch around 006da081,
// mixed_deps_060). Each wrapper builds the 0x64-reserved context on its
// stack, resolves the volume record for the path (006b25b8) and runs the
// worker through 006b2553. The worker and wrapper chains all wait on the
// pool (006b22c3 machinery), so these functions block until completion on
// the owning thread, exactly like the binary.

// 006da081 (cdecl). Context {path, -, out pair-low slot, 0, arg}; after
// the reserved call copies the context word at +8 into *out_result.
void open_file_from_path(const char* path, std::uint32_t* out_result,
                         std::uint32_t arg);

// 006da10c (cdecl). Same shape with {path, a, b, 1, c}; no result copy.
void write_file_flagged(const char* path, std::uint32_t a, std::uint32_t b,
                        std::uint32_t c);

// 006da14e (cdecl). 006da10c with the flag word 1 replaced by zero
// (`AND [EBP-8],0` in the exported x86).
void write_file_plain(const char* path, std::uint32_t a, std::uint32_t b,
                      std::uint32_t c);

// 006da4b1 (cdecl). Four-word context {path, a, b, 1}; answers 1 exactly
// when the reserved call returned 1 (the DEC/NEG/SBB/INC tail is the
// `response == 1` test).
bool check_file_flagged(const char* path, std::uint32_t a, std::uint32_t b);

// 006da4f3 (cdecl). 006da4b1 with the flag word zero.
bool check_file_plain(const char* path, std::uint32_t a, std::uint32_t b);

// ---- reserved-callback workers (cdecl, run by 006b2553) ----------------
// The binary passes the 0x64 reserve value and a pointer to the wrapper's
// stack context through the two words; the second argument is typed here
// like the 006b2553 callback shape (the caller cast in the exported asm).

// Worker of 006da081 (006d9f47). Establishes (or waits for) the bounded
// path record for the context path (mode = reserve), runs the plain op on
// it, hands the plain pair-low to the error-report object DAT_00851384
// (vtable slot 0) with the 18-dword hlsfile.cpp descriptor, and on a
// non-zero answer queues the repeat op carrying (answer, record 2) before
// the final plain wait. Writes the second record into context[2] and
// returns the error-report answer.
std::uint32_t open_file_reserved(std::uint32_t reserve,
                                 std::int32_t context_word);

// Worker of 006da10c/006da14e (006da0c0). Establishes the bounded path
// record (field 1, mode = reserve), queues the repeat op
// (context[1], context[2], mode = reserve-1), waits once more and returns
// the repeat pair-low.
std::uint32_t write_file_reserved(std::uint32_t reserve,
                                  std::int32_t context_word);

// Worker of 006da4b1/006da4f3 (006da462). Establishes the bounded path
// record (field 6, mode = reserve), queues the probe op through 006b17b7
// with (context[1] at +0x30, context[2] as the +0x28 pair, mode =
// reserve-1), waits once more and answers whether the probe pair-low
// equals context[2].
std::uint32_t check_file_reserved(std::uint32_t reserve,
                                  std::int32_t context_word);

} // namespace hp6::runtime
