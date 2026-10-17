#include "platform/Sync.h"

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#endif

// The ownership predicate reads the current-thread id cached in this global;
// its value is not present in the Ghidra export.
extern std::uint32_t DAT_00899acc;

namespace hp6::platform {

void initialize_lock(std::uintptr_t frame) {
#ifdef _WIN32
    InitializeCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(frame + 4));
#else
    (void)frame;
#endif
}

bool create_event(std::uintptr_t frame) {
#ifdef _WIN32
    HANDLE handle = CreateEventA(nullptr, FALSE, FALSE, nullptr);
    if (handle != nullptr) {
        *reinterpret_cast<HANDLE*>(frame + 4) = handle;
        return true;
    }
#else
    (void)frame;
#endif
    return false;
}

void signal_event(std::uintptr_t frame) {
#ifdef _WIN32
    SetEvent(*reinterpret_cast<HANDLE*>(frame + 4));
#else
    (void)frame;
#endif
}

void enter_lock(std::uintptr_t frame) {
#ifdef _WIN32
    EnterCriticalSection(
        reinterpret_cast<LPCRITICAL_SECTION>(frame + 4));
#endif
}

void leave_lock(std::uintptr_t frame) {
#ifdef _WIN32
    LeaveCriticalSection(
        reinterpret_cast<LPCRITICAL_SECTION>(frame + 4));
#endif
}

void sleep_alertable(std::uint32_t milliseconds) {
#ifdef _WIN32
    SleepEx(milliseconds, 1);
#endif
}

bool thread_owns(std::intptr_t frame) {
#ifdef _WIN32
    if (frame == 0) {
        const DWORD current = GetCurrentThreadId();
        return 1 - (current != DAT_00899acc);
    }
    if (frame == -1) {
        return true;
    }
    const DWORD current = GetCurrentThreadId();
    const auto* stored = reinterpret_cast<const std::uint32_t*>(frame + 8);
    return *stored == current;
#else
    (void)frame;
    return false;
#endif
}

// 006d9351. Context header: { wake flag, callback, argument } — the first
// twelve bytes stay on the launcher stack (retained[3] is the unused pad of
// the local fallback storage; the flag lives at storage + 12 in that case).
struct LaunchContext {
    std::uint32_t retained[3];
    volatile std::int32_t wake;
    void (*callback)(void*);
    void* argument;
};

unsigned
#ifdef _MSC_VER
__stdcall
#endif
thread_trampoline_006d9351(void* context) {
    auto* ctx = static_cast<LaunchContext*>(context);
    ctx->wake = 1;
    ctx->callback(ctx->argument);
    return 0;
}

bool launch_thread(std::uintptr_t storage, void (*callback)(void*),
                   void* callback_argument, std::uint32_t /*reserved*/,
                   std::uint32_t stack_size, std::int32_t priority_code) {
#ifdef _WIN32
    // The wake/callback/argument triple always sits on this launcher's own
    // stack; the trampoline reads it from the worker thread after the flag
    // write (same race exists in the binary).
    LaunchContext context{};
    context.wake = 0;
    context.callback = callback;
    context.argument = callback_argument;

    std::uintptr_t frame = storage;
    if (frame == 0) {
        frame = reinterpret_cast<std::uintptr_t>(&context);
    }
    unsigned thread_id = 0;
    const std::uintptr_t thread = _beginthreadex(
        nullptr, stack_size, thread_trampoline_006d9351, &context, 4,
        &thread_id);
    *reinterpret_cast<std::uintptr_t*>(frame + 4) = thread;
    *reinterpret_cast<unsigned*>(frame + 8) = thread_id;
    if (thread == 0) {
        return false;
    }
    set_thread_priority(static_cast<std::intptr_t>(frame), priority_code);
    ResumeThread(*reinterpret_cast<HANDLE*>(frame + 4));
    while (context.wake == 0) {
        SleepEx(1, 1);
    }
    if (storage == 0) {
        CloseHandle(*reinterpret_cast<HANDLE*>(frame + 4));
    }
    return true;
#else
    (void)storage;
    (void)callback;
    (void)callback_argument;
    (void)stack_size;
    (void)priority_code;
    return false;
#endif
}

} // namespace hp6::platform
