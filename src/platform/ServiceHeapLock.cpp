// 0x005DD280, 0x005DD290 and the 0x005DD2A0 thunk to 0x00FF8F70.
// Their Win32 imports and the extra recursion counter at +0x18 are visible
// in the unprotected x86 disassembly and the Ghidra exports.
#include <cstdint>

#ifdef _WIN32
#include <windows.h>

namespace {
std::uint32_t& depth(void* lock) {
    return *reinterpret_cast<std::uint32_t*>(static_cast<std::uint8_t*>(lock) + 0x18);
}
} // namespace

void service_heap_lock_005dd290(void* lock) {
    EnterCriticalSection(static_cast<LPCRITICAL_SECTION>(lock));
    ++depth(lock);
}

void service_heap_unlock_005dd2a0(void* lock) {
    --depth(lock);
    LeaveCriticalSection(static_cast<LPCRITICAL_SECTION>(lock));
}

void service_heap_free_lock_005dd280(void* lock) {
    DeleteCriticalSection(static_cast<LPCRITICAL_SECTION>(lock));
}
#endif
