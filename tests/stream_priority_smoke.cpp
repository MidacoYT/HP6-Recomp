#include "stream/Metadata.h"
#include "stream/ChannelRegistry.h"
#include "runtime/PoolList.h"
#include "platform/Sync.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

// Inert pool infrastructure pulled in transitively through PoolList.obj /
// PoolAccess.obj by hp6::runtime::update_node_priority (see
// tests/pool_list_smoke.cpp for the same set).
std::int32_t DAT_00892b60 = 0;
std::uint32_t DAT_00899acc = 0;
std::int32_t DAT_0085139c = 0;
void* PTR_FUN_007159e8 = nullptr;
void* DAT_006b18de = nullptr;
void* PTR_FUN_00715ac0 = nullptr;
void* PTR_FUN_00715ae0 = nullptr;
void* PTR_FUN_00715ad0 = nullptr;
void* PTR_FUN_00715b00 = nullptr;
void* PTR_FUN_00715af0 = nullptr;
std::int32_t engine_worker_006b1e32(std::int32_t engine) { (void)engine; return 0; }
void* lpTargetHandle_00899ad0 = nullptr;

// Channel table, pulled in through ChannelRegistry.obj.
std::uint8_t DAT_0089b2fe = 0;
std::uint32_t DAT_0089b200 = 0;
std::uint8_t DAT_0089b438 = 0;

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

alignas(16) std::uint8_t header[0x200];
alignas(16) std::uint8_t slot_word[4];
}

int main() {
    using namespace hp6::stream;
    static_assert(sizeof(void*) == 4, "The binary stores 32-bit addresses");

    std::memset(header, 0, sizeof header);
    word(header, 0) = 0x4d525453u; // 'STRM'
    word(slot_word, 0) = address(header);
    const auto* slot = reinterpret_cast<const std::uintptr_t*>(slot_word);

    // Inactive header: the crossing target at +0x54 is still recorded, but
    // no pool-priority update is issued (header+0x44 != 1).
    word(header, 0x44) = 0;
    word(header, 0x58) = 10; // threshold
    word(header, 0x50) = 5;  // initial level, below threshold
    set_header_level(slot, 20); // crosses upward
    expect("inactive_target_recorded", word(header, 0x54) == 1);
    expect("inactive_level_written", word(header, 0x50) == 20);

    // No crossing: the level changes but stays on the same side of the
    // threshold, so the target is left untouched.
    word(header, 0x54) = 0xdeadbeef;
    set_header_level(slot, 25);
    expect("no_crossing_target_unchanged", word(header, 0x54) == 0xdeadbeef);
    expect("no_crossing_level_written", word(header, 0x50) == 25);

    // The calling-convention thunks forward unchanged.
    word(header, 0x50) = 0;
    set_header_level_006f4ffd(slot, 30);
    expect("thunk_006f4ffd_forwards", word(header, 0x50) == 30);
    set_header_level_006f4eb9(slot, 31);
    expect("thunk_006f4eb9_forwards", word(header, 0x50) == 31);

#ifdef _WIN32
    // Active header: a threshold crossing escalates the pool-queue
    // priority of the node whose id is header+0x184, to the byte at
    // header+0x4c.
    alignas(16) std::uint8_t root[0x100];
    alignas(16) std::uint8_t pool_slot[0xa0];
    alignas(16) std::uint8_t node[0x20];
    std::memset(root, 0, sizeof root);
    std::memset(pool_slot, 0, sizeof pool_slot);
    std::memset(node, 0, sizeof node);
    DAT_00892b60 = static_cast<std::int32_t>(address(root));
    word(root, 0x98) = address(pool_slot);
    word(pool_slot, 0x10) = address(node);
    word(pool_slot, 0x14) = address(node);
    word(pool_slot, 0x18) = 1;
    word(node, 0x0c) = 0; // pool index 0
    node[0x12] = 5;       // current priority
    ::InitializeCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(pool_slot + 0x2c));
    expect("queue_event_created", hp6::platform::create_event(
        reinterpret_cast<std::uintptr_t>(pool_slot + 0x44)));
    HANDLE queue_event = *reinterpret_cast<HANDLE*>(pool_slot + 0x48);

    word(header, 0x44) = 1;    // active
    word(header, 0x4c) = 9;    // priority to apply
    word(header, 0x184) = 0;   // pool index
    word(header, 0x58) = 10;
    word(header, 0x50) = 5;
    set_header_level(slot, 20); // crosses upward -> apply_level_target(1)
    expect("active_target_recorded", word(header, 0x54) == 1);
    expect("pool_priority_escalated", node[0x12] == 9);
    expect("queue_signaled", ::WaitForSingleObject(queue_event, 0) == WAIT_OBJECT_0);
    ::CloseHandle(queue_event);
    ::DeleteCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(pool_slot + 0x2c));
#endif

    // Channel registry: table lookup, linked bucket search, and the
    // forwarding chain apply_channel_level -> set_header_level_006f4eb9 ->
    // 006f4ffd -> set_header_level.
    alignas(16) std::uint8_t channel_entry[0x130];
    alignas(16) std::uint8_t channel_node[0x10];
    std::memset(channel_entry, 0, sizeof channel_entry);
    std::memset(channel_node, 0, sizeof channel_node);
    DAT_0089b2fe = 1;
    DAT_0089b200 = address(channel_entry);
    word(channel_entry, 0) = address(slot_word);
    // The id's low byte selects the table bucket (only index 0 is
    // populated above), so the node's own id must share that low byte.
    word(channel_node, 0x0c) = 0x1200u;
    word(channel_entry, 0x120) = address(channel_node);

    expect("channel_lookup", channel_table_entry(0) == address(channel_entry));
    expect("channel_lookup_oob", channel_table_entry(1) == 0);
    expect("channel_node_found", find_channel_node(0x1200u) ==
                                 reinterpret_cast<std::uint32_t*>(channel_node));
    expect("channel_node_missing", find_channel_node(0x1300u) == nullptr);

    DAT_0089b438 = 0;
    expect("channel_disabled", apply_channel_level(0, 5) ==
                               static_cast<std::int32_t>(0xfffffff6u));
    DAT_0089b438 = 1;
    expect("channel_missing_entry", apply_channel_level(1, 5) ==
                                    static_cast<std::int32_t>(0xfffffff8u));

    word(header, 0x44) = 0; // inactive: avoids the pool dependency here
    word(header, 0x58) = 1;
    word(header, 0x50) = 0;
    expect("channel_apply_ok", apply_channel_level(0, 5) == 0);
    expect("channel_apply_wrote_level", word(header, 0x50) == 5);

    std::printf("stream_priority_smoke: %d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
