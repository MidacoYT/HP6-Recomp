#include "render/RenderHeap.h"
#include "platform/HandleRegistry.h"

#include <cstdint>
#include <cstdlib>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
// DAT_00fcdd78 is shared with src/platform/HandleRegistry.cpp's
// service_holder() (which performs the lazy init both globals depend on)
// and src/presentation/TaskSystem.cpp's allocate_tagged.
extern void* DAT_00fcdd74[6];

// Override tables for the two render-framework heap regions; empty (all
// counts 0) in every path reconstructed so far, so every lookup below
// takes its "not found" default. Declared as the binary's own exported
// shape (parallel index/value arrays) rather than invented structs.
extern std::uint32_t DAT_008796b0;      // size-override count
extern std::uint8_t* PTR_DAT_008796b4;  // {uint32 index; int32 size;}[], stride 8
extern std::uint32_t DAT_008796b8;      // flag-override count
extern std::uint8_t* PTR_DAT_008796bc;  // {uint32 index; uint32 flag;}[], stride 8
extern std::uint32_t DAT_008796c8;      // name-override count
extern std::uint8_t* PTR_DAT_008796cc;  // {uint32 index; char* name; uint32 _unused;}[], stride 0xc

namespace hp6::render {
namespace {

#ifdef _MSC_VER
using CreateHeapCall = void* (__thiscall*)(void*, std::int32_t, const void*);
#else
using CreateHeapCall = void* (*)(void*, std::int32_t, const void*);
#endif

// Mirrors the {flag, name, reserved} shape hp6::presentation::
// allocate_tagged already uses on this same function pointer
// (DAT_00fcdd78); only the first two of FUN_006194a0's five stack locals
// (local_c, local_8) are actually passed to this call in the exported
// x86 — the other three (local_18/local_14/local_10, assigned but never
// read afterward here) are preserved below for fidelity even though they
// have no observed effect on this call.
struct HeapCreateDescriptor {
    std::int32_t flag;
    std::int32_t extra;
};

std::int32_t lookup_region_size(std::uint32_t region_index) {
    for (std::uint32_t i = 0; i < DAT_008796b0; ++i) {
        auto* entry = PTR_DAT_008796b4 + i * 8;
        if (*reinterpret_cast<std::uint32_t*>(entry) == region_index) {
            return *reinterpret_cast<std::int32_t*>(entry + 4);
        }
    }
    return 0;
}

std::uint8_t lookup_region_flag(std::uint32_t region_index) {
    for (std::uint32_t i = 0; i < DAT_008796b8; ++i) {
        auto* entry = PTR_DAT_008796bc + i * 8;
        if (*reinterpret_cast<std::uint32_t*>(entry) == region_index) {
            return static_cast<std::uint8_t>(*reinterpret_cast<std::uint32_t*>(entry + 4));
        }
    }
    return 0;
}

const char* lookup_region_name(std::uint32_t region_index) {
    for (std::uint32_t i = 0; i < DAT_008796c8; ++i) {
        auto* entry = PTR_DAT_008796cc + i * 0xc;
        if (*reinterpret_cast<std::uint32_t*>(entry) == region_index) {
            return *reinterpret_cast<char**>(entry + 4);
        }
    }
    return nullptr;
}

} // namespace

std::int32_t create_local_allocator_register(const char*, std::uint32_t, void*, void*,
                                              std::uint32_t, bool) {
    // Fixed-success placeholder until 0x00FF8B60 is integrated. It does
    // not register the sub-allocator created by the original binary.
    return 1;
}

void initialize_render_framework_heap(std::uint8_t* registry) {
    std::int32_t total = 0;
    for (std::uint32_t region = 0; region < 2; ++region) {
        const std::int32_t size = lookup_region_size(region);
        *reinterpret_cast<std::int32_t*>(registry + 0x250 + region * 4) = size;
        total += size + 0x1000;
    }
    *reinterpret_cast<std::int32_t*>(registry + 0x264) = total;

    hp6::platform::service_holder();

    if (total == 0) {
        std::abort();
        return;
    }

    HeapCreateDescriptor descriptor{2, 0x80};
    // Preserved for fidelity; not observed to affect this call (see the
    // struct's doc comment above).
    const std::int32_t unused_field_a = 1;
    const char* const unused_field_b = "RenderFrameworkMainHeap::";
    const std::int32_t unused_field_c = 0;
    (void)unused_field_a;
    (void)unused_field_b;
    (void)unused_field_c;

    auto create_heap = *reinterpret_cast<CreateHeapCall const*>(DAT_00fcdd74[1]);
    void* heap_handle = create_heap(&DAT_00fcdd74[1], total, &descriptor);
    *reinterpret_cast<void**>(registry + 0x260) = heap_handle;
    if (heap_handle == nullptr) {
        std::abort();
        return;
    }

    std::uint8_t* record = registry;
    std::int32_t running_offset = 0;
    for (std::uint32_t region = 0; region < 2; ++region) {
        auto* region_ptr = static_cast<std::uint8_t*>(heap_handle) + running_offset;
        *reinterpret_cast<void**>(registry + 0x258 + region * 4) = region_ptr;
        if (region_ptr == nullptr) {
            std::abort();
            return;
        }
        const std::uint8_t flag = lookup_region_flag(region);
        const char* name = lookup_region_name(region);
        const std::int32_t size = *reinterpret_cast<std::int32_t*>(registry + 0x250 + region * 4);
        const std::int32_t ok = create_local_allocator_register(
            name, static_cast<std::uint32_t>(size), record, region_ptr, 1u << flag, true);
        if (ok == 0) {
            std::abort();
            return;
        }
        record += 0x128;
        running_offset += 0x1000 + size;
    }
}

} // namespace hp6::render
