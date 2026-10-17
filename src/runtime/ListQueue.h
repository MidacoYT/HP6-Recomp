#pragma once

#include <cstdint>

namespace hp6::runtime {

// Intrusive singly-linked FIFO queue shared by the pool and the record
// consumers next to the timer wheel (006b1f02, 006db1a5, 006daf68, ...).
// Header layout: head at +0, tail at +4, count at +8; the next-pointer of
// every linked node lives at node + 0.

// 006db315 (thiscall in the binary, one stack argument cleaned by the
// callee): appends `node` to the queue, clears its next pointer and bumps
// the count.
void queue_push_006db315(std::uint8_t* header, std::uint8_t* node);

// 006db335 (fastcall in the binary, the header in ECX with no stack
// argument): detaches the queue's head node; the count only moves when a
// node existed and a single-node queue clears both the head and the tail.
// The popped node's next pointer ends zeroed; an empty queue is a no-op.
void queue_pop_006db335(std::uint8_t* header);

// 006b2dde (thiscall in the binary): removes one specific node from the
// same queue shape. `previous_hint` may carry the node the caller already
// knows precedes `node` (zero means "search from the head"). Returns 1 on
// success; a missing node (or an empty queue without the node at the
// head) leaves everything untouched. The removed node's link is zeroed.
bool queue_remove_006b2dde(std::uint8_t* header, std::uint8_t* node,
                           std::uint8_t* previous_hint);

} // namespace hp6::runtime
