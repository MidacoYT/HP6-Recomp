// Local behavior checks for the small record/drive-enumeration/timer
// utilities reached from 0x00688130 (not yet reconstructed), one of
// WinMain's early init calls. Game-space globals get test-local
// definitions, the way the other src/platform tests already do.
#include "platform/StorageInit.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

std::int32_t DAT_00851380[0x50 / 4] = {};
std::uint32_t DAT_00892b80[0x68 / 4] = {};
// `extern` is required: a const variable at namespace scope has
// internal linkage by default in C++ (unlike C), which would otherwise
// leave StorageInit.cpp's own `extern` declaration unresolved.
extern const std::uint32_t PTR_DAT_008513e0[26] = {
    1000, 1001, 1002, 1003, 1004, 1005, 1006, 1007, 1008, 1009,
    1010, 1011, 1012, 1013, 1014, 1015, 1016, 1017, 1018, 1019,
    1020, 1021, 1022, 1023, 1024, 1025,
};

// ---- initialize_file_system_storage's own extern dependencies ------------
std::uint8_t DAT_00ced2b0 = 0;
std::uint8_t DAT_00cf1b04 = 0;
std::uint8_t DAT_00853f84 = 0;
std::int32_t DAT_00cf1b00 = 0;

// ---- hp6::platform::service_holder dependencies (pulled in transitively
// through initialize_file_system_storage's own call to it) -----------------
void* PTR_FUN_0083e55c[27] = {};
void* DAT_00fcdd74[6] = {};
void** DAT_00cef6bc = nullptr;
std::uint32_t DAT_00fd1254 = 0;
void cleanup_00711e60() {}
void* PTR_FUN_007159b0 = nullptr;
extern void* PTR_FUN_00715d0c[1] = {};
extern void* PTR_LAB_00715da8[26] = {};
void* PTR_LAB_0071b520 = nullptr;
std::uint32_t DAT_008998c8 = 0;

// ---- hp6::render::create_local_allocator_register dependencies (RenderHeap.cpp) --
std::uint32_t DAT_008796b0 = 0;
std::uint8_t* PTR_DAT_008796b4 = nullptr;
std::uint32_t DAT_008796b8 = 0;
std::uint8_t* PTR_DAT_008796bc = nullptr;
std::uint32_t DAT_008796c8 = 0;
std::uint8_t* PTR_DAT_008796cc = nullptr;

// 0x006d95d0's reconstruction status (see StorageInit.cpp) and its
// fixed callback target.
int register_timer_completion_calls = 0;

namespace hp6::platform {
std::int32_t register_timer_completion(void*, void*, std::int32_t, std::int32_t, std::uint32_t,
                                       std::int32_t) {
    ++register_timer_completion_calls;
    return 0;
}
} // namespace hp6::platform

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}
int popcount32(std::uint32_t value) {
    int count = 0;
    while (value != 0) {
        count += static_cast<int>(value & 1u);
        value >>= 1;
    }
    return count;
}
} // namespace

int main() {
    using namespace hp6::platform;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // ---- reset_oversized_record (006b3411) --------------------------------
    {
        std::int32_t record[0x60 / 4];
        std::memset(record, 0xab, sizeof(record));
        record[0] = 0x60; // > 0x50: the whole record gets zero-filled
        expect("reset_oversized_returns_true", reset_oversized_record(record) == 1);
        bool all_zero = true;
        for (std::int32_t v : record) {
            if (v != 0) all_zero = false;
        }
        expect("reset_oversized_zero_fills", all_zero);
    }
    {
        std::int32_t record[0x60 / 4];
        std::memset(record, 0xab, sizeof(record));
        record[0] = 0x40; // <= 0x50: left untouched by this function
        expect("reset_not_oversized_untouched", reset_oversized_record(record) == 1 &&
                                                 record[1] != 0);
    }

    // ---- initialize_default_record_and_cache_flag (006b343e) --------------
    std::memset(DAT_00851380, 0xcd, sizeof(DAT_00851380));
    std::int32_t small_record[0x60 / 4] = {};
    small_record[0] = 0x40; // < 0x50: triggers the shared default-record zero-fill
    small_record[0xb] = 777;
    expect("initialize_default_zero_fills_shared",
           initialize_default_record_and_cache_flag(small_record) == 777);
    bool shared_all_zero = true;
    for (std::int32_t v : DAT_00851380) {
        if (v != 0) shared_all_zero = false;
    }
    expect("initialize_default_shared_record_zeroed", shared_all_zero);

    // ---- enumerate_logical_drives_into_table (006b3656) --------------------
    std::memset(DAT_00892b80, 0xff, sizeof(DAT_00892b80));
    const DWORD real_drive_mask =
#ifdef _WIN32
        GetLogicalDrives();
#else
        0u;
#endif
    const std::int32_t count = enumerate_logical_drives_into_table();
    expect("enumerate_drive_count_matches_popcount", count == popcount32(real_drive_mask));
    bool table_matches = true;
    std::int32_t slot = 0;
    for (int index = 0; index < 0x1a; ++index) {
        if ((real_drive_mask & (1u << index)) != 0) {
            if (DAT_00892b80[slot] != PTR_DAT_008513e0[index]) table_matches = false;
            ++slot;
        }
    }
    expect("enumerate_drive_table_matches_source", table_matches);

    // ---- zero_fill_record (006d927a) ---------------------------------------
    std::uint8_t record28[0x1c];
    std::memset(record28, 0x5a, sizeof(record28));
    expect("zero_fill_record_returns_same_pointer", zero_fill_record(record28) == record28);
    bool record28_zero = true;
    for (std::uint8_t b : record28) {
        if (b != 0) record28_zero = false;
    }
    expect("zero_fill_record_zeroed", record28_zero);

    // ---- schedule_autosave_timer (006d9d15) --------------------------------
    // A real Win32 waitable timer is created/armed (not mocked); this is a
    // local, harmless timer object this process owns, matching the real
    // registry round-trips elsewhere in this test suite.
    expect("schedule_autosave_timer_returns_interval", schedule_autosave_timer(10) == 10);
    expect("schedule_autosave_timer_registered_completion",
           register_timer_completion_calls == 1);

    if (failures != 0) {
        std::printf("storage_init_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("storage_init_smoke: all checks passed\n");
    return 0;
}
