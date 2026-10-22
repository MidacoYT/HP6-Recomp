#include "platform/ServiceHolderDestructor.h"
#include "platform/HandleRegistry.h"

// 0x005DB9A0 is reconstructed in ServiceHolderCleanup.cpp and invokes the
// service heap teardown reconstructed in ServiceHeapTeardown.cpp.
extern void cleanup_service_holder_005db9a0(void* holder);
extern void service_handle_free_005db8d0(void* block, std::uint32_t mode);

namespace hp6::platform {

void* ServiceHolderObject::destroy(std::uint32_t flags) {
    // Unprotected executable 0x005DB970..0x005DB997. The cleanup call
    // precedes the optional allocation-service release.
    cleanup_service_holder_005db9a0(this);
    if ((flags & 1u) != 0) {
        release_through_service(this);
    }
    return this;
}

void ServiceHolderObject::free_block(void* block, std::uint32_t mode) {
    // 0x005DB8D0 ignores ECX and the second stack argument. The primary
    // vtable still uses the game's thiscall ABI for its slot.
    service_handle_free_005db8d0(block, mode);
}

} // namespace hp6::platform
