#include "platform/Sync.h"

#ifdef _WIN32
#include <windows.h>
#endif

extern void* lpTargetHandle_00899ad0;

namespace hp6::platform {

bool set_thread_priority(std::intptr_t frame, std::int32_t code) {
#ifdef _WIN32
    HANDLE handle = static_cast<HANDLE>(lpTargetHandle_00899ad0);
    if (frame == -1) handle = GetCurrentThread();
    else if (frame != 0) handle = *reinterpret_cast<HANDLE*>(frame + 4);
    if (handle != nullptr) {
        int priority = 0;
        switch (code) {
        case -3: priority = -15; break;
        case -2: priority = -2; break;
        case -1: priority = -1; break;
        case 1: priority = 1; break;
        case 2: priority = 2; break;
        case 3: priority = 15; break;
        default: break;
        }
        SetThreadPriority(handle, priority);
    }
#else
    (void)frame;
    (void)code;
#endif
    return true; // 006d9564 sets AL to 1 on every path.
}

} // namespace hp6::platform
