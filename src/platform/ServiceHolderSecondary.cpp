#include "platform/ServiceHolderDestructor.h"

#include <cstdint>

namespace hp6::platform {

// Secondary service interface begins at holder + 4. These two functions are
// the 0x005E00C0 and 0x005E0080 this-adjusting tail jumps in both PE images.
void ServiceHolderSecondary::free_block(void* block, std::uint32_t mode) {
    auto* holder = reinterpret_cast<ServiceHolderObject*>(
        reinterpret_cast<std::uint8_t*>(this) - 4);
    holder->free_block(block, mode);
}

void* ServiceHolderSecondary::destroy(std::uint32_t flags) {
    auto* holder = reinterpret_cast<ServiceHolderObject*>(
        reinterpret_cast<std::uint8_t*>(this) - 4);
    return holder->destroy(flags);
}

} // namespace hp6::platform
