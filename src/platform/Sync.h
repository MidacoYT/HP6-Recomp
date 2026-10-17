#pragma once

// Runtime lock/sleep primitives pulled out of the HP6 binary exports.
// ReAgent was unavailable for this batch (LLM usage limit); the integrations
// follow the exported decompilations directly. Equivalence remains
// unverified.

#include <cstdint>

namespace hp6::platform {

// 006d92a1. The lock lives at frame + 4; the frame is carried in ECX
// (__fastcall) in the original.
void enter_lock(std::uintptr_t frame);

// 006d928b. Initializes the critical section at frame +4.
void initialize_lock(std::uintptr_t frame);

// 006d92ac. Counterpart leave, same frame layout.
void leave_lock(std::uintptr_t frame);

// 006d92b7 / 006d92da. Auto-reset event held at frame +4.
bool create_event(std::uintptr_t frame);
void signal_event(std::uintptr_t frame);

// 006d94f5. Selects global, current, or frame +4 thread handle, maps the
// seven priority codes, and returns true in AL even when the Win32 call fails.
bool set_thread_priority(std::intptr_t frame, std::int32_t code);

// 006d9416. Alertable wait: SleepEx(milliseconds, 1).
void sleep_alertable(std::uint32_t milliseconds);

// 006d944e. Ownership predicate: owner 0 compares the current thread id to
// the global DAT_00899acc; owner -1 always grants; otherwise the current
// thread id is compared with the DWORD stored at frame + 8.
bool thread_owns(std::intptr_t frame);

// 006d9351. CRT thread trampoline (the _beginthreadex start address of
// 006d95d0): writes 1 into the first context word (the wake flag the
// launcher polls), then invokes the callback held at +4 with the argument
// at +8. stdcall in the binary; the _beginthreadex proc type returns
// unsigned (its value is not read by any caller observed here).
#ifdef _MSC_VER
unsigned __stdcall thread_trampoline_006d9351(void* context);
#else
unsigned thread_trampoline_006d9351(void* context);
#endif

// 006d95d0. Suspended-thread launcher (CREATE_SUSPENDED via the CRT import
// _beginthreadex, start address 006d9351). `storage` receives the handle at
// +4 and the thread id at +8; the starter (callback, argument) rides in a
// launcher-stack context header whose first word is the wake flag polled
// with SleepEx(1, 1). `reserved` is never read by the binary; priority_code
// goes to 006d94f5. Returns a bool in AL (MOV AL,1 / XOR AL,AL in the
// exported x86): true on successful thread creation.
bool launch_thread(std::uintptr_t storage, void (*callback)(void*),
                   void* callback_argument, std::uint32_t reserved,
                   std::uint32_t stack_size, std::int32_t priority_code);

} // namespace hp6::platform
