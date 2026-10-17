// Local behavior checks for batch engine_boot_040: the suspended-thread
// launcher (006d95d0) + CRT trampoline (006d9351, CRT import 006f363c),
// the engine-thread bootstrap (006b203a) and the engine attach constructor
// (006b20e8). Game-space symbols get test-local definitions. Not a binary
// comparison; x86 pointer layout required.
#include "platform/Sync.h"
#include "runtime/PoolAccess.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

std::int32_t DAT_00892b60 = 0;
std::int32_t DAT_0085139c = 0x4000;
std::uint32_t DAT_00899acc = 0;
void* PTR_FUN_007159e8 = nullptr;
void* DAT_006b18de = nullptr;
void* lpTargetHandle_00899ad0 = nullptr;

namespace {

int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

std::uint32_t address(const void* value) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}

std::uint32_t& word(void* bytes, std::size_t offset) {
    return *reinterpret_cast<std::uint32_t*>(
        static_cast<std::uint8_t*>(bytes) + offset);
}

// Engine object whose vtable slot +4 (index 1) is the bootstrap tail call.
int slot_calls = 0;
void* slot_this = nullptr;
struct EngineNode {
    virtual void slot0() {}
    virtual void slot1() { ++slot_calls; slot_this = this; }
    virtual void slot2() {}
    virtual void slot3() {}
};

alignas(16) std::uint8_t root[0x100];
alignas(16) std::uint8_t slots[0x500];

} // namespace

// Engine worker body (006b1e32), not integrated in this batch. Global-scope
// definition: the launcher references this address; the stub signals the
// manual-reset event so the bootstrap waiter proceeds.
int worker_calls = 0;
std::int32_t engine_worker_006b1e32(std::int32_t engine) {
    ++worker_calls;
#ifdef _WIN32
    SetEvent(*reinterpret_cast<HANDLE*>(
        static_cast<std::uintptr_t>(static_cast<std::uint32_t>(engine)) +
        0x78));
#endif
    return 0;
}

int main() {
    using namespace hp6::platform;
    using namespace hp6::runtime;
    static_assert(sizeof(void*) == 4, "The binary stores 32-bit addresses");
#ifdef _WIN32
    std::memset(root, 0, sizeof root);
    DAT_00892b60 = static_cast<std::int32_t>(address(root));
    DAT_0085139c = 0x4000;
    word(root, 0x98) = address(slots);
    word(root, 0x50) = 5;                       // slot-token counter
    initialize_lock(reinterpret_cast<std::uintptr_t>(root + 0xc));   // root boot lock
    initialize_lock(reinterpret_cast<std::uintptr_t>(root + 0x54));  // token lock

    auto* engine = slots + 7 * 0x88;
    auto* engine2 = slots + 8 * 0x88;
    std::memset(slots, 0, sizeof slots);

    // ---- bootstrap_engine early-exit (006b203a, flagged record) --------
    EngineNode node;
    EngineNode node2;
    word(engine, 0x80) = 1;
    word(engine, 0x84) = address(&node);
    bootstrap_engine(reinterpret_cast<std::uintptr_t>(engine));
    expect("boot_flagged_slot", slot_calls == 1 && slot_this == &node);
    expect("boot_flagged_word4", word(engine, 4) == 1);

    // ---- bootstrap_engine full path ------------------------------------
    std::memset(engine, 0, 0x90);
    word(engine, 0x84) = address(&node2);
    slot_calls = 0;
    slot_this = nullptr;
    worker_calls = 0;
    bootstrap_engine(reinterpret_cast<std::uintptr_t>(engine));
    expect("boot_slot_called", slot_calls == 1 && slot_this == &node2);
    expect("boot_worker_ran", worker_calls == 1);
    expect("boot_thread_handle", word(engine, 0x6c) != 0);
    expect("boot_thread_id", word(engine, 0x70) != 0);
    expect("boot_wake_word", word(engine, 0x7c) == 0xff);
    expect("boot_zero_c", word(engine, 0xc) == 0);
    expect("boot_flag_untouched", word(engine, 0x80) == 0);

    // ---- attach_engine (006b20e8) --------------------------------------
    word(engine2, 4) = 1;                       // skip the nested bootstrap
    word(engine2, 0x84) = address(&node);
    alignas(16) std::uint8_t receiver[0x40];
    std::memset(receiver, 0xcd, sizeof receiver);
    const std::uint8_t* returned =
        attach_engine(receiver, 0x2a, 0x1234,
                      reinterpret_cast<std::uintptr_t>(engine2));
    expect("attach_return", returned == receiver);
    expect("attach_vtable",
           *reinterpret_cast<void**>(receiver) == static_cast<void*>(&PTR_FUN_007159e8));
    expect("attach_0x14", word(receiver, 0x14) == 0xffffffff);
    expect("attach_0x12", receiver[0x12] == 0x2a);
    expect("attach_0x10_11", receiver[0x10] == 0 && receiver[0x11] == 0);
    expect("attach_0x18", word(receiver, 0x18) == 0x1234);
    expect("attach_0x1c",
           *reinterpret_cast<void**>(receiver + 0x1c) == static_cast<void*>(&DAT_006b18de));
    expect("attach_0x20_30", word(receiver, 0x20) == 0 && word(receiver, 0x24) == 0 &&
                                word(receiver, 0x28) == 0 && word(receiver, 0x2c) == 0 &&
                                word(receiver, 0x30) == 0);
    // allocate_slot_token: ((engine - base) / 0x88) | previous_counter << 8
    expect("attach_token", word(receiver, 0xc) == (5u << 8 | 8u));
    expect("attach_counter", word(root, 0x50) == 6);

    if (failures != 0) {
        std::printf("engine_boot_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("engine_boot_smoke: all checks passed\n");
    return 0;
#else
    (void)root;
    (void)slots;
    std::printf("engine_boot_smoke: skipped (Windows-only)\n");
    return 0;
#endif
}
