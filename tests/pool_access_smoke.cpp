// Local behavior checks for the pool leaves batch (029). The two game-space
// globals required at binary level (DAT_00892b60, DAT_00899acc) are defined
// here with test-local equivalents; the library keeps the external
// declarations. They do not compare against the binary.
#include "platform/Sync.h"
#include "runtime/PoolAccess.h"

#ifdef _WIN32
#include <windows.h>
#endif

#include <cstdio>
#include <cstring>

// Test-local equivalents of the game globals (binary values unknown).
std::int32_t DAT_00892b60 = 0;
std::uint32_t DAT_00899acc = 0;
// Symbols pulled in by the engine-boot extensions of PoolAccess.obj; this
// test exercises only 006b18df/006b1a2e/006b1910 and needs inert values.
std::int32_t DAT_0085139c = 0;
void* PTR_FUN_007159e8 = nullptr;
void* DAT_006b18de = nullptr;
void* lpTargetHandle_00899ad0 = nullptr;
// Global-scope definition: bootstrap_engine references this address.
std::int32_t engine_worker_006b1e32(std::int32_t engine) {
    (void)engine;
    return 0;
}

namespace {

int failures = 0;

void expect(const char* label, bool condition) {
    if (!condition) {
        ++failures;
        std::printf("FAIL %s\n", label);
    }
}

// 64 KiB service space for pool_slot_address (offsets go to +0x98 and the
// slot stride 0x88 reaches +0x8788 from the base stored at +0x98).
std::uint8_t space[0x80000];

struct PairObject {
    virtual void slot0() {}
    virtual void slot1() {}
    virtual void slot2() {}
    virtual void slot3() {}
    virtual void slot4() {}
    virtual void slot5() {}
    virtual void slot6() {}
    virtual void slot7() {}
    virtual void slot8() {}
    virtual std::uint64_t slot9(std::uint32_t argument) {
        return (static_cast<std::uint64_t>(argument) << 32) | 0xABCD1234ull;
    }
};

} // namespace

int main() {
    using namespace hp6::runtime;
    using namespace hp6::platform;

#ifdef _WIN32
    // Critical-section frame: bytes +4 hold the CRITICAL_SECTION. A separate
    // owner frame carries the DWORD at +8 that thread_owns compares; the two
    // structures are unrelated in the binary and must not share storage.
    std::uint8_t cs_frame[0x40];
    std::uint8_t owner_frame[0x10];
    std::memset(cs_frame, 0, sizeof cs_frame);
    std::memset(owner_frame, 0, sizeof owner_frame);
    ::InitializeCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(cs_frame + 4));
    *reinterpret_cast<DWORD*>(cs_frame + 0x30) = GetCurrentThreadId();
    enter_lock(reinterpret_cast<std::uintptr_t>(cs_frame));
    leave_lock(reinterpret_cast<std::uintptr_t>(cs_frame));
    ::DeleteCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(cs_frame + 4));

    // Owner forms of the predicate.
    DAT_00899acc = GetCurrentThreadId();
    expect("owns_global", thread_owns(0));
    DAT_00899acc = GetCurrentThreadId() ^ 1u;
    expect("not_owns_global", !thread_owns(0));
    expect("owns_any", thread_owns(static_cast<std::intptr_t>(-1)));
    *reinterpret_cast<DWORD*>(owner_frame + 8) = GetCurrentThreadId();
    expect("owns_stored",
           thread_owns(reinterpret_cast<std::intptr_t>(owner_frame)));
    *reinterpret_cast<DWORD*>(owner_frame + 8) = GetCurrentThreadId() ^ 5u;
    expect("not_owns_stored",
           !thread_owns(reinterpret_cast<std::intptr_t>(owner_frame)));

    // sleep_alertable(0) returns immediately without waiting; keep it out
    // of the assertions but call it to prove linkage.
    sleep_alertable(0);
#endif

    // Service root record: DAT_00892b60 holds the pointer to `space` (as an
    // int32 in the original), *(space + 0x98) holds the slot base.
    DAT_00892b60 = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(space));
    const std::uintptr_t slot_base = reinterpret_cast<std::uintptr_t>(&space[0x400]);
    *reinterpret_cast<std::int32_t*>(
        reinterpret_cast<std::uintptr_t>(space) + 0x98) =
        static_cast<std::int32_t>(slot_base);

    expect("slot_plain",
           pool_slot_address(5, false) == slot_base + 5 * 0x88);
    expect("slot_ignore_high", pool_slot_address(5 | 0x100u, false) ==
               slot_base + 5 * 0x88);
    *reinterpret_cast<std::int32_t*>(pool_slot_address(5, false) + 0x80) = 1;
    expect("slot_reset", pool_slot_address(5, true) == slot_base);
    expect("slot_no_reset", pool_slot_address(5, false) ==
               slot_base + 5 * 0x88);

    // A real C++ virtual call tests the ECX object argument on MSVC x86.
    PairObject object;
    std::uint8_t frame_pair[0x20];
    *reinterpret_cast<PairObject**>(frame_pair + 0x10) = &object;
    *reinterpret_cast<std::uint32_t*>(frame_pair + 8) = 0x7FFFFFFFu;
    *reinterpret_cast<std::uint32_t*>(frame_pair + 0x18) = 0x11111111u;
    *reinterpret_cast<std::uint32_t*>(frame_pair + 0x1c) = 0x22222222u;

    std::uint64_t value = 0;
    std::uint32_t words[2]{};
    read_virtual_pair(frame_pair, &value, words);
    expect("pair_value", value ==
               (static_cast<std::uint64_t>(0x7FFFFFFF) << 32 | 0xABCD1234ull));
    expect("pair_lt0", words[0] == 0x11111111u && words[1] == 0x22222222u);

    if (failures != 0) {
        std::printf("pool_access_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("pool_access_smoke: all checks passed\n");
    return 0;
}
