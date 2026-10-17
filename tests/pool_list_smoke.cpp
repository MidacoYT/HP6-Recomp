#include "runtime/PoolList.h"
#include "platform/Sync.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef _WIN32
#include <windows.h>
#endif

std::int32_t DAT_00892b60 = 0;
std::uint32_t DAT_00899acc = 0;


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
    return *reinterpret_cast<std::uint32_t*>(static_cast<std::uint8_t*>(bytes) + offset);
}

alignas(16) std::uint8_t root[0x100];
alignas(16) std::uint8_t slot[0xa0];
alignas(16) std::uint8_t first[0x20], middle[0x20], last[0x20];

int notified = 0, deleted = 0, callback_count = 0;
std::uint32_t callback_key = 0, callback_payload = 0;
std::int32_t callback_code = 0;
#ifdef _MSC_VER
// A free __fastcall function receives the virtual object's ECX argument;
// the delete slot gets its flag on the stack and uses stdcall cleanup.
void __fastcall notify_method(void*) { ++notified; }
std::uint64_t __fastcall read_method(void*) { return 0x12345678abcdef01ull; }
void __stdcall delete_method(int flag) { if (flag == 1) ++deleted; }
#else
void notify_method(void*) { ++notified; }
std::uint64_t read_method(void*) { return 0x12345678abcdef01ull; }
void delete_method(void*, int flag) { if (flag == 1) ++deleted; }
#endif
void callback(std::uint32_t key, std::int32_t code, std::uint32_t payload) {
    ++callback_count; callback_key = key; callback_code = code; callback_payload = payload;
}
std::uintptr_t vtable[4] = {
    reinterpret_cast<std::uintptr_t>(delete_method), 0,
    reinterpret_cast<std::uintptr_t>(read_method),
    reinterpret_cast<std::uintptr_t>(notify_method),
};
}

int main() {
    using namespace hp6::runtime;
    static_assert(sizeof(void*) == 4, "The binary list stores 32-bit addresses");

    std::uint32_t list[3] = {address(first), address(last), 3};
    word(first, 8) = address(middle) + 8;
    word(middle, 8) = address(last) + 8;
    word(last, 8) = 0;
    word(first, 0x0c) = 10;
    word(middle, 0x0c) = 20;
    word(last, 0x0c) = 30;

    expect("find_middle", find_node(reinterpret_cast<std::uint8_t*>(list), 20, false) == middle);
    expect("find_missing", find_node(reinterpret_cast<std::uint8_t*>(list), 40, false) == nullptr);
    expect("detach_middle", detach_node(reinterpret_cast<std::uint8_t*>(list), middle, first));
    expect("middle_relinked", list[2] == 2 && word(first, 8) == address(last) + 8 &&
                                word(middle, 8) == 0);
    expect("detach_missing", !detach_node(reinterpret_cast<std::uint8_t*>(list), middle, first));
    expect("detach_head", detach_first(reinterpret_cast<std::uint8_t*>(list)) == first);
    expect("head_relinked", list[0] == address(last) && list[2] == 1 && word(first, 8) == 0);
    expect("detach_last", find_node(reinterpret_cast<std::uint8_t*>(list), 30, true) == last);
    expect("list_empty", list[0] == 0 && list[1] == 0 && list[2] == 0);

#ifdef _WIN32
    std::memset(root, 0, sizeof root);
    std::memset(slot, 0, sizeof slot);
    DAT_00892b60 = static_cast<std::int32_t>(address(root));
    ::InitializeCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(root + 0x10));
    word(root, 0) = address(first);
    word(root, 4) = address(first);
    word(root, 8) = 1;
    expect("locked_head", detach_first_locked(0x40) == first);
    expect("locked_empty", word(root, 0) == 0 && word(root, 8) == 0);
    ::DeleteCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(root + 0x10));

    word(root, 0x98) = address(slot);
    word(slot, 4) = 1;
    word(slot, 0x1c) = address(last);
    word(slot, 0x20) = address(last);
    word(slot, 0x24) = 1;
    word(last, 0x0c) = 0;
    last[0x10] = 0;
    last[0x11] = 7;
    ::InitializeCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(slot + 0x2c));
    expect("status_second_list", pool_node_status(0) == 7);
    last[0x10] = 1;
    expect("status_busy", pool_node_status(0) == -1);
    word(slot, 0x1c) = 0;
    expect("status_missing", pool_node_status(0) == -3);

    // The event at +0x44 is signaled after each sorted insertion.
    expect("queue_event_created", hp6::platform::create_event(
        reinterpret_cast<std::uintptr_t>(slot + 0x44)));
    HANDLE queue_event = *reinterpret_cast<HANDLE*>(slot + 0x48);
    word(first, 0x0c) = word(middle, 0x0c) = word(last, 0x0c) = 0;
    first[0x12] = 10;
    middle[0x12] = 20;
    last[0x12] = 30;
    word(slot, 0x10) = word(slot, 0x14) = word(slot, 0x18) = 0;
    schedule_node(last);
    schedule_node(first);
    schedule_node(middle);
    expect("sorted_count", word(slot, 0x18) == 3);
    expect("sorted_links", word(slot, 0x10) == address(first) &&
                           word(first, 8) == address(middle) + 8 &&
                           word(middle, 8) == address(last) + 8 &&
                           word(slot, 0x14) == address(last));
    expect("queue_signaled", ::WaitForSingleObject(queue_event, 0) == WAIT_OBJECT_0);
    update_node_priority(0, 40);
    expect("priority_reordered", word(slot, 0x10) == address(middle) &&
                                 word(last, 8) == address(first) + 8 &&
                                 word(slot, 0x14) == address(first) &&
                                 word(slot, 0x18) == 3);
    word(middle, 0) = address(vtable);
    notify_pool_node(0);
    expect("virtual_notify", notified == 1);
    set_pool_callback(0, callback);
    expect("callback_stored", word(middle, 0x1c) == address(
        reinterpret_cast<void*>(callback)));

    word(slot, 0x10) = word(slot, 0x14) = word(slot, 0x18) = 0;
    word(slot, 0x1c) = word(slot, 0x20) = address(last);
    word(slot, 0x24) = 1;
    word(last, 0) = address(vtable);
    word(last, 0x0c) = 0;
    last[0x11] = static_cast<std::uint8_t>(-2);
    word(last, 0x18) = 0xabc;
    set_pool_callback(0, callback);
    expect("callback_called", callback_count == 1 && callback_key == 0 &&
                              callback_code == -2 && callback_payload == 0xabc);
    expect("completed_value", take_completed_node(0) == 0x12345678abcdef01ull);
    expect("completed_deleted", deleted == 1 && word(slot, 0x24) == 0);
    ::CloseHandle(queue_event);
    ::DeleteCriticalSection(reinterpret_cast<LPCRITICAL_SECTION>(slot + 0x2c));
#endif

    std::printf("pool_list_smoke: %d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
