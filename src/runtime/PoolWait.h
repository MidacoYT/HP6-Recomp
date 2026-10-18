#pragma once

#include <cstdint>

namespace hp6::runtime {

// 006b22c3 (cdecl). Resolves the pool slot for `token` (006b18df with
// reset=1) and returns -3 when the slot record is missing (+4 zero).
// Otherwise repeatedly searches the slot for the node keyed by the same
// word under the lock at +0x28 (006b2147), keeping the loop only while the
// search succeeds; between unsuccessful rounds the caller either pumps the
// timer wheel and sleeps one tick (owning thread, 006d971d/006d9416) or
// waits on the manual-reset event at +0x78 and resets it. Returns the
// classification 006b2223 gives for the same key.
std::int32_t wait_pool_node(std::uint32_t token);

// 006b23a9 (cdecl): bare JMP to 006b2359 - the completed-answer passthrough.
std::int64_t completed_state(std::uint32_t token);

// 006b17e5 (cdecl): creates the timed node from the record with no attach
// words (006b2456), waits for its completion (006b22c3), reads the pair
// back (006b2359) and answers 1 exactly when that pair is non-zero.
bool create_and_wait_node(std::uintptr_t record, std::uint8_t mode);

// 006b183a (cdecl): same chain over the volume-lookup named node A
// (006b289e) with no attach words.
bool create_and_wait_named_node(const char* path, std::uint8_t mode);

// 006b175b (cdecl): same chain over the volume-lookup bounded path node
// (006b2990) with no attach words; returns the pair itself.
std::int64_t create_and_wait_bounded_path_node(const char* path,
                                               std::uint32_t field_0x20,
                                               std::uint8_t mode);

// 006b1783 (cdecl): same chain over the repeat creator (006b24b7); the
// signed repeats are delivered as sign-extended pairs (CDQ funnels) with
// no attach words. Returns the pair itself.
std::int64_t create_and_wait_repeat_node(std::uintptr_t record,
                                         std::int32_t value_1,
                                         std::uint32_t word_0x20,
                                         std::int32_t value_2,
                                         std::uint8_t mode);

// 006b1815 (cdecl): same chain over the plain creator (006b2527) with no
// attach words; returns the pair itself.
std::int64_t create_and_wait_plain_node(std::uintptr_t record,
                                        std::uint8_t mode);

// 006b17b7 (cdecl): same chain over the probe creator (006b24f2) with no
// attach words; returns the pair itself. The caller's argument word feeds
// the funnel of 006b24f2 unchanged; the CDQ sign extensions happen inside
// 006b329a. Used by the reserved file-op probes (006da462 machinery).
std::int64_t create_and_wait_probe_node(std::uintptr_t record,
                                        std::int32_t value_0x20,
                                        std::uint32_t field_0x30,
                                        std::int32_t value_0x28,
                                        std::uint8_t mode);

} // namespace hp6::runtime
