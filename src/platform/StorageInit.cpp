#include "platform/StorageInit.h"
#include "platform/HandleRegistry.h"
#include "render/RenderHeap.h"

#include <cstring>
#include <windows.h>

// Addresses read/written by the binary but absent from the exported
// globals; declared with the real sizes the exported x86's zero_fill
// calls operate on (not plain scalars), so a test-provided definition
// doesn't overrun into unrelated memory. Tests provide their own
// definitions the way the other src/platform files already do.
extern std::int32_t DAT_00851380[0x50 / 4];  // shared default-sized record (0x50 bytes)
extern std::uint32_t DAT_00892b80[0x68 / 4]; // per-drive output table, 0x68 bytes
extern const std::uint32_t PTR_DAT_008513e0[26]; // fixed per-drive-letter source table

extern std::uint8_t DAT_00ced2b0;  // copy destination of lpClass_007278d3 (confirmed empty string)
extern std::uint8_t DAT_00cf1b04;  // "file system storage initialized" flag
extern std::uint8_t DAT_00853f84;  // cleared alongside the two flags above
extern std::int32_t DAT_00cf1b00;  // the hardware-compatibility gate's own fast-path flag

namespace hp6::platform {
namespace {

// Purely internal bookkeeping: nothing outside this file reads or
// seeds these, so they need no extern/test-provided storage.
std::int32_t g_cached_record_flag = 0; // DAT_008513d4
void (*g_generic_hook)() = nullptr;    // DAT_008516ac

// 0x008527e0..0x008527f8 (24 bytes) form one contiguous block in the
// exported x86 — hTimer_008527e0 (HANDLE, +0x0), an opaque 12-byte
// context block passed to 0x006d95d0 (DAT_008527e4, +0x4), and the
// LARGE_INTEGER due time (lpDueTime_008527f0 low dword +0x10,
// DAT_008527f4 high dword +0x14) — confirmed by
// schedule_autosave_timer's own 0x18-byte zero-fill of the whole
// block. Modelled as one array rather than four separate globals so
// that zero-fill can't be misread as overrunning unrelated storage.
std::uint8_t g_autosave_timer_block[0x18] = {};
void*& timer_handle() {
    return *reinterpret_cast<void**>(g_autosave_timer_block + 0x0);
}
void* opaque_context() { return g_autosave_timer_block + 0x4; }
void*& due_time_low() {
    return *reinterpret_cast<void**>(g_autosave_timer_block + 0x10);
}
std::int32_t& due_time_high() {
    return *reinterpret_cast<std::int32_t*>(g_autosave_timer_block + 0x14);
}

std::uint8_t g_timer_lazy_init_done = 0; // DAT_00899d5c
std::int32_t g_timer_shared_record[0x20 / 4] = {}; // DAT_00899d60
std::int32_t g_timer_flag_a = 0;         // DAT_00899d84
std::int32_t g_timer_flag_b = 0;         // DAT_00899d88
std::int32_t g_cached_interval = 0;      // DAT_00899d80

} // namespace

// 0x006d95d0 — not reconstructed; a distinct per-timer registration
// helper (6 parameters). Declared, not defined: tracked as an external
// dependency the way other not-yet-resolved callees are elsewhere in
// this project.
extern std::int32_t register_timer_completion(void* context, void* callback, std::int32_t,
                                               std::int32_t, std::uint32_t, std::int32_t);
// LAB_006d9cae — the fixed callback passed to register_timer_completion
// above; not reconstructed, same status.
extern void autosave_timer_callback();

void zero_fill(void* ptr, std::int32_t size) {
    std::memset(ptr, 0, static_cast<std::size_t>(size));
}

void invoke_generic_hook() {
    if (g_generic_hook != nullptr) {
        g_generic_hook();
    }
}

std::uint8_t reset_oversized_record(std::int32_t* record) {
    if (record[0] > 0x50) {
        zero_fill(record, record[0]);
    }
    invoke_generic_hook();
    return 1;
}

std::int32_t initialize_default_record_and_cache_flag(std::int32_t* record) {
    if (record[0] < 0x50) {
        zero_fill(DAT_00851380, 0x50);
    }
    invoke_generic_hook();
    g_cached_record_flag = record[0xb];
    return g_cached_record_flag;
}

std::int32_t enumerate_logical_drives_into_table() {
    const DWORD drive_mask = GetLogicalDrives();
    zero_fill(DAT_00892b80, 0x68);

    std::int32_t count = 0;
    for (int index = 0; index < 0x1a; ++index) {
        if ((drive_mask & (1u << index)) != 0) {
            DAT_00892b80[count] = PTR_DAT_008513e0[index];
            ++count;
        }
    }
    return count;
}

void* zero_fill_record(void* record) {
    zero_fill(record, 0x1c);
    return record;
}

std::int32_t schedule_autosave_timer(std::int32_t interval_divisor_hz) {
    if (g_timer_lazy_init_done == 0) {
        zero_fill(g_timer_shared_record, 0x20);
        zero_fill(g_autosave_timer_block, 0x18);
        g_timer_flag_b = 0;
        g_timer_lazy_init_done = 1;
    }
    g_timer_flag_a = 0;
    timer_handle() = CreateWaitableTimerA(nullptr, TRUE, nullptr);

    const auto due_time =
        static_cast<std::int32_t>(-10000000LL / static_cast<std::int64_t>(interval_divisor_hz));
    due_time_low() = reinterpret_cast<void*>(static_cast<std::intptr_t>(due_time));
    due_time_high() = due_time >> 31;

    register_timer_completion(opaque_context(), reinterpret_cast<void*>(&autosave_timer_callback),
                              0, 0, 0, 2);
    g_cached_interval = interval_divisor_hz;

    SetWaitableTimer(timer_handle(), reinterpret_cast<LARGE_INTEGER*>(&due_time_low()), 0, nullptr,
                     nullptr, FALSE);
    return g_cached_interval;
}

void initialize_file_system_storage() {
    DAT_00cf1b04 = 1;
    DAT_00853f84 = 0;
    DAT_00cf1b00 = 0;
    DAT_00ced2b0 = 0; // lpClass_007278d3 is confirmed empty; the copy is a single NUL byte.

    // NOT YET INTEGRATED: FUN_006d9377(), a documented deferred no-op
    // (purpose not traced; takes no visible arguments here).

    schedule_autosave_timer(100);

    std::int32_t probe_record[0x50 / 4] = {};
    probe_record[0] = 0x50;
    reset_oversized_record(probe_record);

    service_holder();

    // NOT YET INTEGRATED: the real "FileSysMainHeap" backing allocation
    // (a heap-factory call through the service holder's second slot)
    // is deferred as a documented nullptr region base;
    // create_local_allocator_register is itself already a neutral
    // no-op stub (see RenderHeap.cpp), so this changes nothing
    // observable.
    hp6::render::create_local_allocator_register(
        "FileSysMainHeap", 0x3e800, reinterpret_cast<void*>(0x891e38), nullptr, 0x80, true);

    std::int32_t default_record[21] = {};
    default_record[0] = 0x80;
    initialize_default_record_and_cache_flag(default_record);

    // NOT YET INTEGRATED: FUN_006b2a23(nullptr, 0), a documented
    // deferred no-op.

    // The trio gated on WinMain's own bootup-service object
    // (FUN_006daee1/FUN_006b1f02/FUN_006b1b21) is honestly unreachable
    // here: that object (WinMain's DAT_00cef698) is itself a
    // documented deferred nullptr (see platform/WinMain.cpp), so this
    // function's own equivalent gate never fires, matching the
    // exported x86's own behaviour whenever that object fails to
    // create.
}

} // namespace hp6::platform
