// Both verified HP6 PEs store 0x00D63EE0 at 0x00892990. The object at
// 0x00D63EE0 has zero words at +0, +4, +8 and +0xC initially; the listener
// routines use +8 as their free-list head and +0xC as their live-node count.
#include <cstdint>

std::uint32_t DAT_00d63ee0[4] = {};
void* DAT_00892990 = DAT_00d63ee0;
