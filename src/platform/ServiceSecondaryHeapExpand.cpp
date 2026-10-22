#include "platform/Diagnostics.h"

#include <cstdint>

extern void* DAT_00cef6b0;
extern void* DAT_00cef6b8;
extern std::uint32_t DAT_00cef700;
extern std::uint32_t DAT_00cef70c;
extern void* DAT_00fcaea8;
extern void* service_heap_allocate_core_005dbc50(
    void* heap, const char* context, std::uint32_t alignment,
    std::uint32_t size, std::uint32_t region, std::uint32_t flags);
extern std::uint32_t service_heap_allocate_00ff9850(
    void* heap, void* region, std::uint32_t size, std::uint8_t mode,
    std::uint32_t, std::uint32_t, std::uint32_t);
extern void service_heap_release_region_005de540(void* heap, void* region);

std::uint32_t service_secondary_heap_expand_005dbbb0(std::uint32_t request) {
    // 0x005DBBB0..0x005DBC4D. EAX supplies request. New regions are rounded
    // to one quarter of the initial service block, then allocated from the
    // secondary heap and attached to the primary heap.
    const std::uint32_t quantum = DAT_00cef700 >> 2;
    const std::uint32_t size = ((request - 1u) / quantum + 1u) * quantum;
    void* region = service_heap_allocate_core_005dbc50(
        DAT_00cef6b8, "GlobalAllocator::Core", 0x10, size, 0, 0);
    if (region == nullptr) return 0;
    if ((service_heap_allocate_00ff9850(
             DAT_00cef6b0, region, size, 0, 0, 0, 0) & 0xffu) == 0) {
        service_heap_release_region_005de540(DAT_00cef6b8, region);
        return 0;
    }
    DAT_00cef70c += size;
    constexpr const char* message =
        "GlobalAllocator: Allocated extra core Memory Block size %d\n";
    if (DAT_00fcaea8 != nullptr) {
        auto* vtable = *static_cast<void***>(DAT_00fcaea8);
        using Log = void (*)(void*, const char*, std::uint32_t);
        reinterpret_cast<Log>(vtable[0])(DAT_00fcaea8, message, size);
    } else {
        hp6::platform::format_diagnostic_message(message, size);
    }
    return 1;
}
