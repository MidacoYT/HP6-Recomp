#pragma once
#include <cstdint>

namespace hp6::platform {

// 0x006b3b0e (cdecl). Zero-fills `size` bytes at `ptr` through the
// imported CRT memset slot (PTR_FUN_008516a8); std::memset is
// behaviourally identical and does not need that indirection
// reproduced.
void zero_fill(void* ptr, std::int32_t size);

// 0x006b3b04 (stdcall, no arguments). Calls a runtime-settable global
// hook (DAT_008516ac, kept file-local here since nothing outside this
// module reads or sets it) with no arguments; a no-op until something
// else (not yet reconstructed) installs a real handler there.
void invoke_generic_hook();

// 0x006b3411 (cdecl). `record[0]` is the record's own recorded size;
// when it exceeds the 0x50-byte default inline capacity, zero-fills
// the whole record (zero_fill(record, record[0])) before invoking the
// generic hook. Always returns 1.
std::uint8_t reset_oversized_record(std::int32_t* record);

// 0x006b343e (cdecl). The companion check to reset_oversized_record:
// when `record[0]` is still below the 0x50-byte default capacity,
// zero-fills the shared default-sized record at DAT_00851380 instead
// of this one. Always invokes the generic hook, then caches
// `record[0xb]` into DAT_008513d4 and returns it (only the low byte of
// the exported x86's return value is defined, matching the project's
// established convention for such otherwise-unconstrained upper
// bytes; this returns the plain cached value).
std::int32_t initialize_default_record_and_cache_flag(std::int32_t* record);

// 0x006b3656 (stdcall, no arguments). Zero-fills a 0x68-byte table at
// DAT_00892b80, then copies PTR_DAT_008513e0[i] (a fixed, 26-entry
// per-drive-letter table) into consecutive slots of that table for
// every drive letter GetLogicalDrives() reports present. Returns the
// number of drives found.
std::int32_t enumerate_logical_drives_into_table();

// 0x006d927a (fastcall). Zero-fills a fixed 0x1c-byte record and
// returns the same pointer unchanged.
void* zero_fill_record(void* record);

// 0x006d9d15 (cdecl). One-shot lazily zero-fills two shared records
// (DAT_00899d60, hTimer_008527e0) on the first call, then (re)creates
// a manual-reset Win32 waitable timer (CreateWaitableTimerA) due to
// fire after `interval_divisor_hz` divisions of one second
// (-10000000 / interval_divisor_hz, in 100ns units — a negative
// LARGE_INTEGER due time is relative to now) and arms it
// (SetWaitableTimer) with a completion routine registered through
// 0x006d95d0 (not reconstructed; a distinct per-timer registration
// helper, tracked as an external dependency) and the fixed callback at
// LAB_006d9cae. Returns `interval_divisor_hz` unchanged. Reached from
// 0x00688130 (not yet reconstructed), alongside
// reset_oversized_record/initialize_default_record_and_cache_flag —
// most likely a periodic-autosave-trigger setup, matching the
// "Storage.CreateTimedAutosaveTrigger"/"Storage.Autosave" presentation
// commands (see architecture/startup_path.md), though that link is not
// yet confirmed by cross-reference.
std::int32_t schedule_autosave_timer(std::int32_t interval_divisor_hz);

// 0x00688130. Called once from WinMain, right after the particle-table
// and debug-variable-system inits and right before the hardware
// compatibility gate (0x00ff8ce0) — which reads DAT_00cf1b00, the flag
// this function unconditionally clears to 0, making that gate's inner
// (untraced) compatibility check provably unreachable in this call
// chain. Reuses reset_oversized_record, initialize_default_record_and_
// cache_flag and schedule_autosave_timer(100) (all already integrated
// above), plus hp6::render::create_local_allocator_register (itself
// already a neutral no-op stub) for the "FileSysMainHeap" registration.
// The real heap-factory call backing that registration (a double-
// indirected call through the service holder's second slot) and three
// leaf helpers (FUN_006d9377, FUN_006b2a23, and the
// FUN_006daee1/FUN_006b1f02/FUN_006b1b21 trio gated on WinMain's own
// still-untraced bootup-service object) are documented deferred no-ops.
void initialize_file_system_storage();

} // namespace hp6::platform
