#pragma once

#include <cstdint>

// 0x005DD4C0 in the unprotected x86 image. The three stack arguments follow
// the heap pointer passed in EAX by the original caller.
std::uint32_t service_heap_initialize_005dd4c0(
    void* heap, std::uint32_t request, std::uint32_t size, std::uint8_t mode);
