#pragma once
#include <cstdint>

namespace hp6::stream {

// Returns 0 for a block beginning with 'STRM' and 1 otherwise.
// The two outputs are the address of the slot and the header it points to.
std::uint32_t locate_header(const std::uintptr_t* slot,
                            std::uintptr_t* slot_address,
                            std::uintptr_t* header_address); // 0x006db3f9

std::uint32_t slot_word_04(const std::uintptr_t* slot); // 0x006dbb8e
std::uint32_t slot_word_08(const std::uintptr_t* slot); // 0x006dbda1
std::uint32_t header_word_44(const std::uintptr_t* slot); // 0x006dbdc6
std::int32_t header_delta_40_38(const std::uintptr_t* slot); // 0x006dbe1d
std::uint32_t header_word_58(const std::uintptr_t* slot); // 0x006dbe45

std::uintptr_t find_entry(std::uintptr_t header_address,
                          std::uint32_t identifier); // 0x006db50a
void set_descriptor(const std::uintptr_t* slot, std::int32_t index,
                    std::int32_t first, std::int32_t second,
                    std::int32_t tag); // 0x006dba7f
void set_header_words_48_4c(const std::uintptr_t* slot,
                            std::uint32_t first,
                            std::uint32_t second); // 0x006dbaed
std::uintptr_t indexed_record_10(const std::uintptr_t* slot,
                                 std::int32_t index); // 0x006dbb55
std::uint32_t is_header_inactive(const std::uintptr_t* slot); // 0x006dbdeb
std::uint32_t entry_value(const std::uintptr_t* slot,
                          std::uint32_t identifier); // 0x006dbee7

// 0x006dc17c: writes the level word at header+0x50, then calls
// apply_level_target when the threshold comparison at header+0x58 flips
// (the level crosses the threshold in either direction).
void set_header_level(const std::uintptr_t* slot, std::int32_t level);

// 0x006dbb17: records the crossing direction at header+0x54. When the
// header is active (header+0x44 == 1) and the target is non-zero, escalates
// the pool-queue priority of the node at header+0x184 to the byte at
// header+0x4c (hp6::runtime::update_node_priority).
void apply_level_target(const std::uintptr_t* slot, std::int32_t target);

// 0x006f4ffd / 0x006f4eb9: calling-convention-normalizing tail-call stubs
// (PUSH EBP; MOV EBP,ESP; POP EBP; JMP), each forwarding its two arguments
// unchanged to the next. Reached only from
// hp6::stream::apply_channel_level (ChannelRegistry.cpp).
void set_header_level_006f4ffd(const std::uintptr_t* slot, std::int32_t level);
void set_header_level_006f4eb9(const std::uintptr_t* slot, std::int32_t level);

}
