#pragma once

#include <cstdint>

namespace hp6::presentation {

// Common vtable slots 1..3 in all 26 records at 0x0087BB8C..0x0087BCB8.
struct NativeScreenRecord {
    void* vtable;
    const char* name;
    void* pending_task;

    void cancel_pending_task(); // 0x0050BB50
    void clear_pending_task();  // 0x0050C0C0
    bool has_task(void* task);   // 0x0050C140
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(NativeScreenRecord) == 12);
#endif

} // namespace hp6::presentation
