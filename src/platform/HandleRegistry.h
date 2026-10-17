#pragma once

// Service-holder and handle-release paths from the HP6 exports. ReAgent was
// unavailable for this batch (LLM usage limit); integrations follow the
// exported decompilations. Equivalence remains unverified.
//
// The four constructor-style functions each install a distinct imported
// vtable pointer (0x007159b0, 0x00715d0c, 0x00715da8, 0x0071b520); the
// tables themselves belong to the game's address space and are referenced
// through external declarations only — no class is defined here.

#include <cstdint>

namespace hp6::platform {

// 006b936d. CRT __onexit thunk: success maps to 0, failure to -1.
std::int32_t atexit_register(void (*handler)(void));

// 005db920. One-shot initialization of the static service holder: sets the
// lazy flag DAT_00fd1254 bit 0, installs the two vtable tables, feeds the
// global watch pointer DAT_00cef6bc, and registers cleanup_00711e60 through
// the CRT thunk. Returns the holder address every call.
void** service_holder();

// 005dc710. Forwards the object to virtual service slot +0xc (index 3) of
// the static holder, passing (object, 0); no-op on null.
void release_through_service(void* object);

// 006d91f0. Handle close: pointers outside the static block [0x00896498,
// 0x00899697] go through release_through_service; in-block pointers zero
// a table word under LOCK before the CloseHandle. The handle at *pointers
// is closed only when non-null (original branch order preserved).
void close_handle(void* pointer);

// 006b2e49 / 006b4a02 / 006b7cef / 006dcef4. Constructor-style functions
// (ECX this-passing in the original): install the given vtable pointer at
// *this and, when flag bit 0 is set, release the fresh object through the
// service slot. Each address keeps a distinct entry.
void* construct_vtable_007159b0(void* target, std::uint8_t flag);
void* construct_vtable_00715d0c(void* target, std::uint8_t flag);
void* construct_vtable_00715da8(void* target, std::uint8_t flag);
void* construct_vtable_0071b520(void* target, std::uint8_t flag);

} // namespace hp6::platform
