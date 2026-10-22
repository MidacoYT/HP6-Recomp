#include <cstdint>
#include <windows.h>

// 0x005DD830 operation 0, with ECX=1 at the only call in 0x005DD4C0.
// Jump-table slot 0 enters 0x005DD854. It points the lock at the embedded
// 0x4E8 critical section after the 0x005DD260 -> 0x00FF8F50 initializer.
void service_heap_enable_lock_005dd830_0(void* heap_ptr) {
    auto* heap = static_cast<std::uint8_t*>(heap_ptr);
    auto& lock = *reinterpret_cast<void**>(heap + 0x4e4);
    if (lock != nullptr) return;
    auto* embedded = reinterpret_cast<LPCRITICAL_SECTION>(heap + 0x4e8);
    *reinterpret_cast<std::uint32_t*>(heap + 0x500) = 0;
    InitializeCriticalSectionAndSpinCount(embedded, 10);
    lock = embedded;
}
