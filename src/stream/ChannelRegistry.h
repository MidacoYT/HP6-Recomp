#pragma once
#include <cstdint>

namespace hp6::stream {

// 0x006e0778: byte-indexed lookup into a shared channel table. The table
// base is DAT_0089b200 (one 32-bit entry pointer per slot); the live entry
// count is the byte at DAT_0089b2fe. Returns 0 for an out-of-range index.
std::uint32_t channel_table_entry(std::int32_t index);

// 0x006e3f71: within the bucket selected by channel_table_entry(id & 0xff),
// walk the singly linked list rooted at the bucket entry's word +0x120
// (next pointer at node+0, identifier at node+0xc) for a node whose
// identifier equals the full id. Returns nullptr when absent or the bucket
// is empty.
std::uint32_t* find_channel_node(std::uint32_t id);

// 0x006e71e1: resolve a channel table entry to its stream slot (the word at
// entry+0) and apply a level through set_header_level. Returns 0 on
// success, -8 (0xfffffff8) when the table entry is empty, and -10
// (0xfffffff6) when the channel system is disabled (DAT_0089b438 == 0).
std::int32_t apply_channel_level(std::int32_t index, std::int32_t level);

}
