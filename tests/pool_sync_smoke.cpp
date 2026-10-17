#include "platform/Sync.h"
#include "runtime/PoolAccess.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

std::int32_t DAT_00892b60 = 0;
std::uint32_t DAT_00899acc = 0;
void* lpTargetHandle_00899ad0 = nullptr;


// Inert definitions for the engine-boot externs newly pulled in through
// PoolAccess.obj; these tests do not exercise the bootstrap path.
std::int32_t DAT_0085139c = 0;
void* PTR_FUN_007159e8 = nullptr;
void* DAT_006b18de = nullptr;
// Node-constructor vtables pulled in through PoolList.obj; inert here.
void* PTR_FUN_00715ac0 = nullptr;
void* PTR_FUN_00715ae0 = nullptr;
void* PTR_FUN_00715ad0 = nullptr;
void* PTR_FUN_00715b00 = nullptr;
std::int32_t engine_worker_006b1e32(std::int32_t engine) {
    (void)engine;
    return 0;
}
namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}
std::uint32_t address(const void* value) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}
std::uint32_t& word(void* bytes, std::size_t offset) {
    return *reinterpret_cast<std::uint32_t*>(static_cast<std::uint8_t*>(bytes) + offset);
}
alignas(16) std::uint8_t root[0x100];
alignas(16) std::uint8_t slots[0x300];
}

int main() {
    using namespace hp6::platform;
    using namespace hp6::runtime;
    static_assert(sizeof(void*) == 4, "The binary stores 32-bit addresses");
#ifdef _WIN32
    std::memset(root, 0, sizeof root);
    DAT_00892b60 = static_cast<std::int32_t>(address(root));
    word(root, 0x98) = address(slots);
    initialize_lock(reinterpret_cast<std::uintptr_t>(root + 0x54));

    word(root, 0x50) = 5;
    expect("slot_token", allocate_slot_token(address(slots) + 2 * 0x88) ==
                         ((5u << 8) | 2u));
    expect("counter_increment", word(root, 0x50) == 6);
    word(root, 0x50) = 0xffff;
    expect("slot_token_wrap", allocate_slot_token(address(slots)) == (0xffffu << 8));
    expect("counter_wrap", word(root, 0x50) == 1);
    ::DeleteCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(root + 0x58));

    alignas(16) std::uint8_t event_frame[0x20]{};
    expect("event_created", create_event(reinterpret_cast<std::uintptr_t>(event_frame)));
    HANDLE event = *reinterpret_cast<HANDLE*>(event_frame + 4);
    signal_event(reinterpret_cast<std::uintptr_t>(event_frame));
    expect("event_signaled", ::WaitForSingleObject(event, 0) == WAIT_OBJECT_0);
    expect("event_auto_reset", ::WaitForSingleObject(event, 0) == WAIT_TIMEOUT);
    ::CloseHandle(event);

    expect("priority_null_global", set_thread_priority(0, 3));
    expect("priority_current", set_thread_priority(-1, 0));
#endif
    std::printf("pool_sync_smoke: %d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
