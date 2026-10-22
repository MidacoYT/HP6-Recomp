#include "platform/HandleRegistry.h"
#include <atomic>
#include <cstdlib>

#ifdef _WIN32
#include <windows.h>
#endif

// Game address-space symbols exported by Ghidra without their contents.
// Their binary values are only meaningful inside the game image.
extern std::uint32_t DAT_00fd1254;          // lazy-init flag (bit 0 used)
extern void* DAT_00fcdd74[6];               // holder words +0x00..+0x14
extern void** DAT_00cef6bc;                 // watcher -> &holder
extern void* PTR_FUN_0083e55c[9];           // two adjacent vtables, 4 + 5 slots
extern void cleanup_00711e60();             // registered de initializer
extern void* PTR_FUN_00715d0c[1];
extern void* PTR_LAB_00715da8[26];
extern std::uint32_t DAT_008998c8;          // zero-store table base

namespace {

#ifdef _MSC_VER
using service_fn = void (__thiscall*)(void* holder, void* object,
                                     std::uint32_t mode);
#else
using service_fn = void (*)(void* holder, void* object, std::uint32_t mode);
#endif

service_fn service_slot(std::int32_t byte_offset) {
    // *holder is the object pointer; the slot function lives at object + 0xc
    // and is dereferenced there, exactly as the decompile does with
    // (**(code **)(*holder + 0xc)).
    return *reinterpret_cast<service_fn*>(
        reinterpret_cast<std::uint8_t*>(DAT_00fcdd74[0]) + byte_offset);
}

} // namespace

namespace hp6::platform {

// 006b936d. The CRT __onexit returns the previous handler or null on
// failure; the binary maps that to (non-null) - 1: 0 on success, -1 on
// failure. Declared through a CRT-side call in the original; reconstructed
// here as a documented thunk.
std::int32_t atexit_register(void (*handler)(void)) {
    // The CRT __onexit semantics (previous hander or null) are documented
    // for parity; the observable mapping reconstructed here is 0 on
    // success and -1 on failure.
    return std::atexit(handler) == 0 ? 0 : -1;
}

void** service_holder() {
    if ((DAT_00fd1254 & 1u) == 0u) {
        DAT_00fd1254 |= 1u;
        DAT_00fcdd74[0] = PTR_FUN_0083e55c;
        DAT_00fcdd74[1] = PTR_FUN_0083e55c + 4;
        DAT_00cef6bc = DAT_00fcdd74;
        atexit_register(&cleanup_00711e60);
    }
    return DAT_00fcdd74;
}

void release_through_service(void* object) {
    if (object != nullptr) {
        service_holder();
        // (**holder + 0xc) is slot index 3 of the service vtable.
        service_fn slot = service_slot(0xc);
        slot(DAT_00fcdd74, object, 0);
    }
}

void close_handle(void* pointer) {
#ifdef _WIN32
    auto* entry = static_cast<std::uint32_t*>(pointer);
    const auto address = reinterpret_cast<std::uintptr_t>(entry);
    if (address < 0x00896498u || address >= 0x00899698u) {
        if (entry == nullptr) return;
        if (*entry != 0u) {
            CloseHandle(reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(*entry)));
        }
        release_through_service(entry);
        return;
    }
    // In-block pointer: zero a table word with the LOCKed store, then close
    // only when the handle is non-null — branch order per decompile.
    const auto slot_index = static_cast<std::uint32_t>(
        (address - 0x00896498u) / 100u);
    std::atomic_ref<std::uint32_t>(
        *(&DAT_008998c8 + slot_index)).exchange(
            0u, std::memory_order_seq_cst);
    if (*entry != 0u) {
        CloseHandle(reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(*entry)));
    }
#endif
}

void* construct_vtable_00715d0c(void* target, std::uint8_t flag) {
    *static_cast<void***>(target) = PTR_FUN_00715d0c;
    if ((flag & 1u) != 0u) {
        release_through_service(target);
    }
    return target;
}

void* construct_vtable_00715da8(void* target, std::uint8_t flag) {
    *static_cast<void***>(target) = PTR_LAB_00715da8;
    if ((flag & 1u) != 0u) {
        release_through_service(target);
    }
    return target;
}

} // namespace hp6::platform
