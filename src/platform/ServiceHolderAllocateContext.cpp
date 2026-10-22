#include "platform/ServiceHolderDestructor.h"

#include <cstdint>

extern void* DAT_00cef6b0;
extern void service_holder_heap_initialize_005dba40(void* holder);
extern const char* service_context_name_005db680(char* destination,
                                                const char* name);
extern const char* service_context_registry_name_005db380(const char* name);
extern void* service_heap_allocate_locked_005dddb0(
    void* heap, std::uint32_t size, std::uint32_t flags);
extern std::uint32_t service_heap_usable_end_offset_005dede0(
    void* heap, void* payload);

// 0x00CEF710: null in both verified PE images.
void* DAT_00cef710 = nullptr;

namespace hp6::platform {

void* ServiceHolderObject::allocate_context(
    std::uint32_t size, const char* name, std::uint32_t flags) {
    // 0x005DB710..0x005DB79A. This variant reserves four trailing bytes
    // for the canonical context pointer after rounding the size to four.
    service_holder_heap_initialize_005dba40(this);
    if (name == nullptr) name = "Unnamed::";
    char context_buffer[0x100];
    const char* context = service_context_name_005db680(context_buffer, name);
    if (DAT_00cef710 != nullptr)
        context = service_context_registry_name_005db380(context);
    else
        context = "CONTEXTREGISTRYUNAVAILABLE";

    const std::uint32_t aligned = (size + 3u) & ~3u;
    void* heap = DAT_00cef6b0;
    void* payload = service_heap_allocate_locked_005dddb0(
        heap, aligned + 4u, flags);
    if (payload != nullptr) {
        const std::uint32_t end = service_heap_usable_end_offset_005dede0(
            heap, payload);
        *reinterpret_cast<const char**>(
            static_cast<std::uint8_t*>(payload) + end - 4) = context;
    }
    return payload;
}

} // namespace hp6::platform
