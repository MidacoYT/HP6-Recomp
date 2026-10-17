#pragma once

#include <cstdint>

namespace hp6::platform {

// One 20-byte timer slot. The binary stores sixteen of them consecutively
// at 0x00899ad8 (the wheel spans 0x00899ad8..0x00899c23, probed by the
// exported loop of 006d971d with a 0x14 stride ending at 0x899c24):
// callback first, then the argument word, the period, the due tick and the
// busy word. Callbacks are callee-cleaned with two stack arguments
// (cdecl): (argument, overdue = now - due).
struct TimerSlot {
    void (*callback)(std::int32_t argument, std::int32_t overdue);
    std::int32_t argument;
    std::int32_t period;
    std::int32_t due;
    std::int32_t busy;
};

// Fixed-address game-space symbols: the wheel, the current tick clock and
// the last-tick snapshot written at the head of each pump. Tests define
// these symbols at global scope; the library only reads/writes them.
extern TimerSlot timer_slots_00899ad8[16];

} // namespace hp6::platform

extern std::int32_t DAT_00899d88;
extern std::int32_t DAT_00899c1c;

namespace hp6::platform {

// 006d971d (stdcall, no arguments). When the calling thread owns the
// global root (thread_owns(0)), snapshots the tick clock into
// DAT_00899c1c, then walks the wheel: a slot fires when its callback is
// set, the scheduler tick has reached its due time (signed compare) and it
// is not busy. Firing sets busy, calls callback(argument, now - due),
// reschedules due to the CURRENT clock plus the period (the assembly
// re-reads DAT_00899d88 after the call) and clears busy.
void pump_timer_slots();

} // namespace hp6::platform
