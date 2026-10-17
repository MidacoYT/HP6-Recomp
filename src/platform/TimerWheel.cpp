#include "platform/TimerWheel.h"
#include "platform/Sync.h"

namespace hp6::platform {

void pump_timer_slots() {
    if (!thread_owns(0)) {
        return;
    }
    DAT_00899c1c = DAT_00899d88;
    for (auto* slot = timer_slots_00899ad8;
         slot < timer_slots_00899ad8 + 16; ++slot) {
        if (slot->callback == nullptr) {
            continue;
        }
        if (DAT_00899d88 < slot->due) {
            continue;
        }
        if (slot->busy != 0) {
            continue;
        }
        const std::int32_t overdue = DAT_00899d88 - slot->due;
        slot->busy = 1;
        slot->callback(slot->argument, overdue);
        // The clock is re-read after the callback, exactly as the exported
        // x86 reloads [0x00899d88] before adding the period.
        slot->due = DAT_00899d88 + slot->period;
        slot->busy = 0;
    }
}

} // namespace hp6::platform
