#include "platform/ServiceHolderDestructor.h"
#include "platform/ServiceHeapConstruct.h"

#include <cstdint>
#include <cstdlib>
#include <initializer_list>

extern void* DAT_00cef6b0;
extern void* DAT_00cef6b4;
extern void* DAT_00cef6b8;
extern void service_heap_dispatch_005dd830_1(void* heap);
extern void service_heap_dispatch_005dd830_2(void* heap, std::uint32_t value);
extern void service_heap_enable_lock_005dd830_0(void* heap);
extern std::uint32_t service_primary_heap_growth_005db640(
    void* heap, std::uint32_t size, std::uint32_t reserve, void* context);
extern std::int32_t command_return_one_0045df40(const char*);

// 0x00CEF700..0x00CEF70F: four zero dwords in both verified PE images.
std::uint32_t DAT_00cef700 = 0;
std::uint32_t DAT_00cef704 = 0;
std::uint32_t DAT_00cef708 = 0;
std::uint32_t DAT_00cef70c = 0;

namespace {
// The heap controls at 0x0089D990, 0x0089E0A8, 0x0089E5C0 occupy at least
// 0x518 bytes each. The first two addresses are 0x718 and 0x518 apart.
alignas(8) std::uint8_t secondary_control_0089d990[0x518] = {};
alignas(8) std::uint8_t tertiary_control_0089e0a8[0x518] = {};
alignas(8) std::uint8_t primary_control_0089e5c0[0x518] = {};

void* construct(void* control, void* backing, std::uint32_t size) {
    return service_heap_construct_005dd2d0(
        control, static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(backing)), size);
}
} // namespace

void service_holder_heap_initialize_005dba40(void* holder_ptr) {
    // 0x005DBA40..0x005DBBAC, after its x86 SEH prologue. The lazy guard is
    // the primary heap pointer, not the holder's vtable.
    if (DAT_00cef6b0 != nullptr) return;
    auto* holder = static_cast<hp6::platform::ServiceHolderObject*>(holder_ptr);
    DAT_00cef704 = 0;
    DAT_00cef708 = 0;
    DAT_00cef70c = 0;
    holder->allocation_at_0c = nullptr;
    holder->allocation_at_10 = nullptr;
    holder->allocation_at_14 = nullptr;
    DAT_00cef700 = 0x1ffc0000;
    void* backing = std::malloc(DAT_00cef700);
    holder->allocation_at_0c = backing;
    DAT_00cef6b0 = construct(primary_control_0089e5c0, backing, DAT_00cef700);
    if (holder->allocation_at_10 != nullptr)
        DAT_00cef6b4 = construct(secondary_control_0089d990,
                                  holder->allocation_at_10, DAT_00cef704);
    if (holder->allocation_at_14 != nullptr)
        DAT_00cef6b8 = construct(tertiary_control_0089e0a8,
                                  holder->allocation_at_14, DAT_00cef708);
    for (void* heap : {DAT_00cef6b0, DAT_00cef6b4, DAT_00cef6b8}) {
        if (heap == nullptr) continue;
        service_heap_dispatch_005dd830_1(heap);
        service_heap_dispatch_005dd830_2(heap, 0);
        service_heap_enable_lock_005dd830_0(heap);
    }
    auto* primary = static_cast<std::uint8_t*>(DAT_00cef6b0);
    *reinterpret_cast<void**>(primary + 0x4ac) =
        reinterpret_cast<void*>(&service_primary_heap_growth_005db640);
    *reinterpret_cast<void**>(primary + 0x4b0) = primary;
    if (DAT_00cef6b4 != nullptr) {
        auto* secondary = static_cast<std::uint8_t*>(DAT_00cef6b4);
        *reinterpret_cast<void**>(secondary + 0x4ac) =
            reinterpret_cast<void*>(&command_return_one_0045df40);
        *reinterpret_cast<void**>(secondary + 0x4b0) = secondary;
    }
}
