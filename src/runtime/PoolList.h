#pragma once

#include <cstdint>

namespace hp6::runtime {

// The binary stores each list as three 32-bit words: first, last, count.
// A node's word at +8 points to the next node's word at +8 (or zero).
// The surrounding object's field meanings are still unknown.

// 006b2cbd: detach the first node, leaving its +8 link clear.
std::uint8_t* detach_first(std::uint8_t* list);

// 006b2c65 / 006b2ce9: insert at head, or immediately after a node.
void insert_first(std::uint8_t* list, std::uint8_t* node);
void insert_after(std::uint8_t* list, std::uint8_t* previous,
                  std::uint8_t* node);

// 006b1d53: lock the global pool list, detach its first node, unlock. All
// observed callers push a size word (cdecl, cleaned by the caller, e.g.
// `PUSH 0x40; CALL; POP ECX` in 006b2456) which the callee never reads.
std::uint8_t* detach_first_locked(std::uint32_t allocation_size);

// 006b2d1d: detach a selected node. Only AL is defined on every return path.
bool detach_node(std::uint8_t* list, std::uint8_t* node,
                 std::uint8_t* previous);

// 006b1ffc: find a node by the 32-bit word at +0xc, optionally detaching it.
std::uint8_t* find_node(std::uint8_t* list, std::int32_t key, bool detach);

// 006b2147: search the current node, then lists at +0x10 and +0x1c.
// Source code: 0 current, 1 first list, 0 second list/missing,
// 0xffffffff current-node match. The returned pointer is preserved in EAX.
std::uint8_t* find_pool_node(std::uint8_t* frame, std::int32_t key,
                             std::uint32_t* source_code);

// 006b2223: classify a pool node under the lock at frame +0x28.
std::int32_t pool_node_status(std::uint32_t index);

// 006b2199 / 006b2408: sorted insertion and priority update of a queued node.
void schedule_node(std::uint8_t* node);
void update_node_priority(std::uint32_t index, std::uint8_t priority);

// 006b227b / 006b2359 / 006b23b2: pool lookup wrappers. Their virtual
// slots (+0xc, +8, +0) and callback arguments are explicit in the x86 code.
void notify_pool_node(std::uint32_t index);
std::uint64_t take_completed_node(std::uint32_t index);
using NodeCallback = void (*)(std::uint32_t, std::int32_t, std::uint32_t);
void set_pool_callback(std::uint32_t index, NodeCallback callback);

// 006b3032 (thiscall in the binary). Engine attach (006b20e8), vtable
// 00715ac0, record word at +0x14, then 006b2199.
std::uint8_t* construct_timer_node(std::uint8_t* receiver, std::uint32_t id,
                                   std::uint8_t mode, std::uint32_t words,
                                   std::uintptr_t engine);

// 006b318b (thiscall in the binary). Engine attach, +0x38/+0x3c cleared,
// words at +0x14/+0x28/+0x2c/+0x20/+0x24, vtable 00715ae0, +0x30 last.
std::uint8_t* construct_repeat_node(
    std::uint8_t* receiver, std::uint32_t word_0x14, std::uint32_t field_0x28,
    std::uint32_t field_0x2c, std::uint32_t field_0x20,
    std::uint32_t field_0x24, std::uint32_t field_0x30, std::uint8_t mode,
    std::uint32_t attach_words, std::uintptr_t engine);

// 006b2456 (cdecl). Pooled 0x40-byte slot + 006b3032, returns the node's
// +0xc word; 0 when the pool is empty.
std::uint32_t create_node_from_record(std::uintptr_t record, std::uint8_t mode,
                                      std::uint32_t words);

// 006b24b7 (cdecl). Pooled 0x40-byte slot + 006b318b, decompiled argument
// funnel (param_1, param_5, param_6, param_2, param_3, param_4, param_7,
// param_8, record + 0xc).
std::uint32_t create_repeat_node_from_record(
    std::uintptr_t record, std::uint32_t field_0x28, std::uint32_t field_0x2c,
    std::uint32_t field_0x20, std::uint32_t field_0x24, std::uint32_t field_0x30,
    std::uint8_t mode, std::uint32_t attach_words);

// 006b2482 (cdecl). 0x40 slot pull + 006b3094 signed constructor, funnel
// (record, bound_high, field_0x30, bound_low, mode, attach_words,
// record + 0xc).
std::uint32_t create_signed_node_from_record(
    std::uintptr_t record, std::uint32_t bound_high, std::uint32_t field_0x30,
    std::uint32_t bound_low, std::uint8_t mode, std::uint32_t attach_words);

// 006b2527 (cdecl). 0x40 slot pull + 006b335c plain constructor with the
// record + 0xc engine.
std::uint32_t create_plain_node_from_record(std::uintptr_t record,
                                            std::uint8_t mode,
                                            std::uint32_t attach_words);

// 006b3094 (thiscall in the binary). Engine attach, +0x38/+0x3c cleared,
// record word at +0x14, bounded words at +0x28/+0x2c and +0x20/+0x24 with
// their sign words (param >> 31), vtable 00715ad0, +0x30, then scheduling.
std::uint8_t* construct_signed_node(std::uint8_t* receiver, std::uint32_t id,
                                    std::int32_t bound_low,
                                    std::int32_t bound_high,
                                    std::uint32_t field_0x30,
                                    std::uint8_t mode,
                                    std::uint32_t attach_words,
                                    std::uintptr_t engine);

// 006b335c (thiscall in the binary). Engine attach first, then the vtable
// 00715b00 BEFORE the record word at +0x14 (unlike the other constructors),
// then scheduling.
std::uint8_t* construct_plain_node(std::uint8_t* receiver, std::uint32_t id,
                                   std::uint8_t mode,
                                   std::uint32_t attach_words,
                                   std::uintptr_t engine);

// 006b329a (thiscall in the binary). Engine attach first, record word at
// +0x14, one signed argument per pair at +0x28/+0x2c and +0x20/+0x24 (the
// sign words come from the CDQ of the int32 argument), vtable 00715af0 and
// the unsigned word at +0x30 last, then scheduling.
std::uint8_t* construct_probe_node_00715af0(
    std::uint8_t* receiver, std::uint32_t record, std::int32_t value_0x28,
    std::int32_t value_0x20, std::uint32_t field_0x30, std::uint8_t mode,
    std::uint32_t attach_words, std::uintptr_t engine);

// 006b24f2 (cdecl). Pooled 0x40-byte slot + 006b329a, decompiled argument
// funnel: 006b329a(slot, param_1, param_4, param_2, param_3, param_5,
// param_6, *(param_1 + 0xc)); returns the node's +0xc word; 0 when the
// pool is empty.
std::uint32_t create_probe_node_from_record(std::uintptr_t record,
                                            std::int32_t value_0x20,
                                            std::uint32_t field_0x30,
                                            std::int32_t value_0x28,
                                            std::uint8_t mode,
                                            std::uint32_t attach_words);

} // namespace hp6::runtime
