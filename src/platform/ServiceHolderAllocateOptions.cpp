#include "platform/ServiceHolderDestructor.h"

#include <cstdint>

extern void* DAT_00cef6b0;
extern void* DAT_00cef6b4;
extern void service_holder_heap_initialize_005dba40(void* holder);
extern const char* service_context_name_005db680(char* destination,
                                                const char* name);
extern void* service_heap_allocate_core_005dbc50(
    void* heap, const char* context, std::uint32_t alignment,
    std::uint32_t size, std::uint32_t region, std::uint32_t flags);

namespace hp6::platform {
namespace {
struct AllocationOption {
    std::uint32_t code;
    std::uint32_t value;
    const AllocationOption* next;
};
static_assert(sizeof(AllocationOption) == 12);
} // namespace

void* ServiceHolderSecondary::allocate_from_options(
    std::uint32_t size, const void* options_ptr) {
    // 0x005DB800..0x005DB8AE. The 0x100-byte stack buffer, default values,
    // option codes and last-option heap selection are read from its x86 body.
    auto* holder = reinterpret_cast<ServiceHolderObject*>(
        reinterpret_cast<std::uint8_t*>(this) - 4);
    service_holder_heap_initialize_005dba40(holder);

    const char* name = nullptr;
    std::uint32_t alignment = 0x10;
    std::uint32_t region = 0;
    std::uint32_t flags = 0;
    void* heap = DAT_00cef6b0;
    const auto* option = static_cast<const AllocationOption*>(options_ptr);
    for (; option != nullptr && option->code != 0; option = option->next) {
        heap = option->code == 0x10000 ? DAT_00cef6b4 : DAT_00cef6b0;
        switch (option->code) {
        case 1:
            name = reinterpret_cast<const char*>(
                static_cast<std::uintptr_t>(option->value));
            break;
        case 2: alignment = option->value; break;
        case 3: region = option->value; break;
        case 4: flags = option->value; break;
        default: break;
        }
    }
    if (name == nullptr) name = "Unnamed::";
    if (heap == nullptr) return nullptr;

    char context_buffer[0x100];
    const char* context = service_context_name_005db680(context_buffer, name);
    return service_heap_allocate_core_005dbc50(
        heap, context, alignment, size, region, flags);
}
} // namespace hp6::platform
