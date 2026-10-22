#include <cstdint>
#include <windows.h>

// 0x005DD2C0..0x005DD2CF. The import at 0x00713064 is OutputDebugStringA.
void service_heap_free_callback_005dd2c0(void* message) {
    if (message != nullptr)
        OutputDebugStringA(static_cast<const char*>(message));
}

// 0x005DDC80..0x005DDC9E: emit the pointer at argument+0x0C, a newline,
// then execute the original INT3 diagnostic break.
void service_heap_release_callback_005ddc80(void* record) {
    const char* message = *reinterpret_cast<const char* const*>(
        static_cast<const std::uint8_t*>(record) + 0xc);
    if (message != nullptr) OutputDebugStringA(message);
    OutputDebugStringA("\n");
    DebugBreak();
}
