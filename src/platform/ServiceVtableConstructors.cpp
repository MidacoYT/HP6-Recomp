#include "platform/HandleRegistry.h"

#include <cstdint>

// Vtables used by two distinct object families. Their methods are not
// required by service_holder's startup path; the final native image must
// supply their relocated arrays before either constructor is called.
extern void* PTR_FUN_007159b0;
extern void* PTR_LAB_0071b520;

namespace hp6::platform {

void* construct_vtable_007159b0(void* target, std::uint8_t flag) {
    *static_cast<void***>(target) = &PTR_FUN_007159b0;
    if ((flag & 1u) != 0u) {
        release_through_service(target);
    }
    return target;
}

void* construct_vtable_0071b520(void* target, std::uint8_t flag) {
    *static_cast<void***>(target) = &PTR_LAB_0071b520;
    if ((flag & 1u) != 0u) {
        release_through_service(target);
    }
    return target;
}

} // namespace hp6::platform
