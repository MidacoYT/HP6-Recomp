#pragma once
#include <cstdint>

namespace hp6::render {

// 0x00ff8b60 ("LocalAllocator_CreateRegister_Overlay" in the independent
// IDA knowledge base at D:\HP\knowledge\hp6). NOT YET INTEGRATED: creates a
// named sub-allocator ("LocalAllocator::<name>::Allocator") through the
// already-integrated named allocation service (hp6::presentation::
// allocate_tagged / hp6::platform::service_holder) and then constructs and
// configures an object on it via three more unresolved functions
// (0x005dd2d0, 0x005dd4c0, three calls to the jump-table dispatcher
// 0x005dd830) wrapped in MSVC x86 SEH. `record` receives a 0x128-byte
// allocator-record struct; `region_base`/`size` describe the memory region
// this sub-allocator manages; `flags` is `1 << capability_bit`. Returns
// non-zero on success in the exported x86. The definition in
// RenderHeap.cpp exists only so dependents link; it aborts loudly if ever
// actually reached, rather than silently faking the real registration.
std::int32_t create_local_allocator_register(const char* name, std::uint32_t size,
                                              void* record, void* region_base,
                                              std::uint32_t flags, bool mark_flag);

// 0x006194a0 (stdcall, one argument: `registry`, called as
// FUN_006194a0(0xd71250) from FUN_005dca10 — 0x00d71250 is a fixed global
// address, not a size, holding this whole structure). Lays out two
// 0x128-byte LocalAllocator records at `registry+0`/`registry+0x128`,
// two region sizes at `registry+0x250`/`+0x254` (looked up from the
// (empty, in this reconstruction) DAT_008796b0/DAT_008796b4 override
// table, defaulting to 0), their start pointers within the heap at
// `registry+0x258`/`+0x25c`, the heap handle at `registry+0x260`, and the
// total requested size at `registry+0x264`. One-time registers an atexit
// cleanup hook (DAT_00fd1254 bit 0) and configures the global heap-vtable
// pointers DAT_00fcdd74/DAT_00fcdd78 the first time this runs. Creates the
// heap itself through `*DAT_00fcdd78` (an engine-supplied heap-creation
// function pointer; the final binary must supply its target, like every
// other not-yet-identified function pointer in this project) sized to the
// total, then registers both regions through
// create_local_allocator_register. Matches the exported x86's own
// behavior exactly on failure: an INT3 (reproduced here as std::abort())
// if the heap or either region fails to create — this project's registry
// is the real "RenderFrameworkMainHeap" object, not an invented stand-in.
void initialize_render_framework_heap(std::uint8_t* registry);

} // namespace hp6::render
