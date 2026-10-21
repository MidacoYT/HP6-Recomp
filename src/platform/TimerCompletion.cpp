// FUN_006D95D0, _StartAddress_006D9351 and the reached branch of
// FUN_006D94F5, reconstructed from the original x86 decompilation.
#include <process.h>
#include <windows.h>

#include <atomic>
#include <cstdint>

namespace hp6::platform {
namespace {

struct ThreadStartRecord {
    std::atomic<std::int32_t> started;
    void (*callback)(void*);
    void* argument;
};
static_assert(sizeof(ThreadStartRecord) == 12);

unsigned __stdcall run_timer_thread(void* opaque) {
    auto* record = static_cast<ThreadStartRecord*>(opaque);
    const auto callback = record->callback;
    void* const argument = record->argument;
    record->started.store(1, std::memory_order_release);
    callback(argument);
    return 0;
}

int native_priority(std::int32_t priority) {
    switch (priority) {
    case -3: return THREAD_PRIORITY_IDLE;
    case -2: return THREAD_PRIORITY_LOWEST;
    case -1: return THREAD_PRIORITY_BELOW_NORMAL;
    case 1: return THREAD_PRIORITY_ABOVE_NORMAL;
    case 2: return THREAD_PRIORITY_HIGHEST;
    case 3: return THREAD_PRIORITY_TIME_CRITICAL;
    default: return THREAD_PRIORITY_NORMAL;
    }
}

} // namespace

std::int32_t register_timer_completion(void* context, void* callback,
                                       std::int32_t argument, std::int32_t /*unused*/,
                                       std::uint32_t stack_size, std::int32_t priority) {
    // The original stores {handle, id} at offsets +4 and +8 of the supplied
    // 12-byte context. With a null context it uses a local one and closes
    // that temporary handle after the worker reports it has started.
    std::uint8_t local_context[12] = {};
    auto* destination = static_cast<std::uint8_t*>(context);
    if (destination == nullptr) destination = local_context;

    ThreadStartRecord start{
        0, reinterpret_cast<void (*)(void*)>(callback),
        reinterpret_cast<void*>(static_cast<std::intptr_t>(argument))
    };
    auto* thread_id = reinterpret_cast<unsigned*>(destination + 8);
    const auto raw_handle = _beginthreadex(nullptr, stack_size, &run_timer_thread,
                                           &start, CREATE_SUSPENDED, thread_id);
    *reinterpret_cast<std::uintptr_t*>(destination + 4) = raw_handle;
    if (raw_handle == 0) return 0;

    const auto handle = reinterpret_cast<HANDLE>(raw_handle);
    SetThreadPriority(handle, native_priority(priority));
    ResumeThread(handle);
    while (start.started.load(std::memory_order_acquire) == 0) SleepEx(1, TRUE);
    if (context == nullptr) CloseHandle(handle);
    return 1;
}

} // namespace hp6::platform
