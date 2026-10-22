#include "presentation/NativeScreenRecordMethods.h"

extern void* DAT_00fcadf0;

namespace hp6::presentation {

void NativeScreenRecord::cancel_pending_task() {
    if (pending_task == nullptr) return;
    // 0x0050BB50..0x0050BB7E: the task handler's list head is at +0xC;
    // each node links from +4 and carries the task pointer at +8.
    auto* handler = static_cast<std::uint8_t*>(DAT_00fcadf0);
    auto* node = *reinterpret_cast<std::uint8_t**>(handler + 0xc);
    while (node != nullptr) {
        if (*reinterpret_cast<void**>(node + 8) == pending_task) {
            *(static_cast<std::uint8_t*>(pending_task) + 4) = 1;
            break;
        }
        node = *reinterpret_cast<std::uint8_t**>(node + 4);
    }
    pending_task = nullptr;
}

void NativeScreenRecord::clear_pending_task() {
    pending_task = nullptr;
}

bool NativeScreenRecord::has_task(void* task) {
    return pending_task == task;
}

} // namespace hp6::presentation
