// Full 0x005DEC90 entry. It is kept separately so the verified nonzero
// call path from 0x005DD6B0 does not pull the unrelated 0x005DD4C0 branch
// into the native link before that initializer is reconstructed.
#include <cstdint>
#include "platform/ServiceHeapInitialize.h"

extern void service_heap_prepare_release_after_flag_005dec90(void* heap);

void service_heap_prepare_release_005dec90(void* heap) {
    const auto bucket_bytes = *reinterpret_cast<const std::uint32_t*>(
        static_cast<const std::uint8_t*>(heap) + 4);
    if (bucket_bytes == 0) {
        service_heap_initialize_005dd4c0(heap, 0, 0, 1);
    } else {
        service_heap_prepare_release_after_flag_005dec90(heap);
    }
}
