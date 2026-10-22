#pragma once

#include <cstdint>

// 0x005DD6B0, called with the heap pointer in ECX in the original x86.
// The low return byte is 1 on every exit path.
std::uint32_t teardown_service_heap_005dd6b0(void* heap);
