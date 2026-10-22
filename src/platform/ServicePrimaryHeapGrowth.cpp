#include "platform/HandleRegistry.h"

#include <cstdint>

extern void* DAT_00cef6b0;
extern void* DAT_00cef6b8;
extern std::uint32_t service_secondary_heap_expand_005dbbb0(std::uint32_t size);

std::uint32_t service_primary_heap_growth_005db640(
    void* heap, std::uint32_t size, std::uint32_t, void* context) {
    // 0x005DB640..0x005DB67B. The caller at 0x005DE252 pushes heap, size,
    // size+0x40, and callback context, in that order. This callback accepts
    // only the primary service heap and expands it via the secondary heap.
    if (context == nullptr || context != DAT_00cef6b0) {
        for (;;) *reinterpret_cast<volatile std::uint32_t*>(0) = 0;
    }
    (void)heap;
    hp6::platform::service_holder();
    if (DAT_00cef6b8 == nullptr) {
        for (;;) *reinterpret_cast<volatile std::uint32_t*>(0) = 0;
    }
    if (service_secondary_heap_expand_005dbbb0(size) != 0) return 1;
    for (;;) *reinterpret_cast<volatile std::uint32_t*>(0) = 0;
}
