#include "platform/ServiceHolderDestructor.h"

#include <cstdint>

extern void* DAT_00cef6b0;
extern void service_holder_heap_initialize_005dba40(void* holder);
extern const char* service_context_name_005db680(char* destination,
                                                const char* name);
extern void* service_heap_allocate_core_005dbc50(
    void* heap, const char* context, std::uint32_t alignment,
    std::uint32_t size, std::uint32_t region, std::uint32_t flags);

namespace hp6::platform {

void* ServiceHolderObject::allocate_named(
    std::uint32_t size, const char* name, std::uint32_t flags,
    std::uint32_t alignment, std::uint32_t region) {
    // 0x005DB7A0..0x005DB7F0: initialize the service, form the context
    // string into the 0x100-byte stack buffer, then pass the five original
    // arguments to 0x005DBC50. EAX carries the context and EDI the heap.
    service_holder_heap_initialize_005dba40(this);
    char context_buffer[0x100];
    const char* context = service_context_name_005db680(context_buffer, name);
    return service_heap_allocate_core_005dbc50(
        DAT_00cef6b0, context, alignment, size, region, flags);
}

} // namespace hp6::platform
