#pragma once

#include <cstdint>

// 0x005DD2D0: ESI carries heap in the x86 binary; the two arguments are
// passed on the stack and forwarded to 0x005DD4C0 with mode 1.
void* service_heap_construct_005dd2d0(void* heap, std::uint32_t request,
                                       std::uint32_t size);
