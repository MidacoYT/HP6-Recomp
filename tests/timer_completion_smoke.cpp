#include <windows.h>

#include <atomic>
#include <cstdint>

namespace hp6::platform {
std::int32_t register_timer_completion(void*, void*, std::int32_t,
                                       std::int32_t, std::uint32_t, std::int32_t);
}

namespace {
std::atomic<std::int32_t> observed{0};

void timer_callback(void* argument) {
    observed.store(static_cast<std::int32_t>(reinterpret_cast<std::intptr_t>(argument)),
                   std::memory_order_release);
}
}

int main() {
    static_assert(sizeof(void*) == 4);
    std::uint32_t context[3] = {};
    if (hp6::platform::register_timer_completion(
            context, reinterpret_cast<void*>(&timer_callback), 0x1234,
            0, 0, 2) != 1) return 1;
    const auto handle = reinterpret_cast<HANDLE>(static_cast<std::uintptr_t>(context[1]));
    if (handle == nullptr) return 2;
    const DWORD result = WaitForSingleObject(handle, 5000);
    CloseHandle(handle);
    if (result != WAIT_OBJECT_0) return 3;
    if (context[2] == 0) return 4;
    if (observed.load(std::memory_order_acquire) != 0x1234) return 5;
    return 0;
}
