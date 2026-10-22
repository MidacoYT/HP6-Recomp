#pragma once

#include <cstdint>

namespace hp6::platform {

// Six 32-bit words of the static holder at 0x00FCDD74, as read and cleared
// by 0x005DB9A0. The heap objects referenced from it are tracked separately.
struct ServiceHolderObject {
    void* primary_vtable;
    void* secondary_vtable;
    void* reserved_at_08;
    void* allocation_at_0c;
    void* allocation_at_10;
    void* allocation_at_14;

    // Vtable 0x0083E55C slot 0, function 0x005DB970.
    void* destroy(std::uint32_t flags);

    // Vtable 0x0083E55C slot 3, function 0x005DB8D0.
    void free_block(void* block, std::uint32_t mode);

    // 0x005DB7A0 (five stack words) and 0x005DB710 (three stack words).
    void* allocate_named(std::uint32_t, const char*, std::uint32_t,
                         std::uint32_t, std::uint32_t);
    void* allocate_context(std::uint32_t, const char*, std::uint32_t);

    // 0x005DB9A0, reached by the CRT exit thunk at 0x00711E60.
    void cleanup();
};

// Interface pointer stored at holder + 4; the methods adjust back to the
// owning holder as the binary thunks at 0x005E00C0/0x005E0080 do.
struct ServiceHolderSecondary {
    void* secondary_vtable;
    // 0x005DB800 (two stack words).
    void* allocate_from_options(std::uint32_t, const void*);
    void free_block(void* block, std::uint32_t mode);
    void* destroy(std::uint32_t flags);
};

#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(ServiceHolderObject) == 0x18);
#endif

} // namespace hp6::platform
