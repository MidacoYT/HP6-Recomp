// Local behavior checks for batch pool_wait_043: the timer wheel tick
// (006d971d) and the shared node waiter (006b22c3) plus the two node
// constructors (006b3032 / 006b318b) and record-fed creators (006b2456 /
// 006b24b7). Game-space symbols get test-local definitions. Not a binary
// comparison; x86 pointer layout required.
#include "platform/TimerWheel.h"
#include "platform/Sync.h"
#include "runtime/PoolAccess.h"
#include "runtime/PoolList.h"
#include "runtime/PoolPath.h"
#include "runtime/PoolWait.h"
#include "runtime/FileOps.h"
#include "runtime/ListQueue.h"
#include "runtime/WrapperStubs.h"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <windows.h>

std::int32_t DAT_00892b60 = 0;
std::uint32_t DAT_00899acc = 0;
std::int32_t DAT_0085139c = 0;
extern "C" std::int32_t DAT_00851390 = 0;  // search path entry count
extern "C" std::int32_t DAT_00851394 = 0;  // search path buffer capacity
void* PTR_FUN_007159e8 = nullptr;
void* DAT_006b18de = nullptr;
void* lpTargetHandle_00899ad0 = nullptr;
std::int32_t engine_worker_006b1e32(std::int32_t engine) {
    (void)engine;
    return 0;
}

std::int32_t DAT_00899d88 = 0;
std::int32_t DAT_00899c1c = -1;
void* PTR_FUN_00715ac0 = nullptr;
void* PTR_FUN_00715ae0 = nullptr;
void* PTR_FUN_00715ad0 = nullptr;
void* PTR_FUN_00715b00 = nullptr;
void* PTR_FUN_00715a90 = nullptr;
void* PTR_FUN_00715aa0 = nullptr;
void* PTR_FUN_00715ab0 = nullptr;
void* PTR_FUN_00715af0 = nullptr;
void* DAT_00851384 = nullptr;
std::int32_t DAT_008513ac = 0;
extern const char empty_string_007278d3[1] = "";
extern const char node_source_path_00715a30[96] =
    "Z:\\hbp\\code\\0707_graphics\\x-platform\\packages\\realcore\\6.27.01"
    "\\source\\file\\cmn\\filesys.cpp";
extern const char hlsfile_source_0071b358[] =
    "Z:\\hbp\\code\\0707_graphics\\x-platform\\packages\\realcore\\6.27.01"
    "\\source\\file\\cmn\\hlsfile.cpp";
hp6::platform::TimerSlot hp6::platform::timer_slots_00899ad8[16] = {};

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

alignas(16) std::uint8_t root[0x100];
alignas(16) std::uint8_t slots[0x500];

// Timer callbacks.
int fires = 0;
std::int32_t fire_arg = -1;
std::int32_t fire_overdue = -1;
void fire(std::int32_t argument, std::int32_t overdue) {
    ++fires;
    fire_arg = argument;
    fire_overdue = overdue;
}

// Error-report object reached through DAT_00851384: vtable slot +0 answers
// the allocation request (thiscall, callee-cleaned stack arguments,
// matching the exported convention of the 006b1968 virtual call).
struct PathAllocator {
    std::uint32_t got_length = 0;
    const void* got_request = nullptr;
    // Lot 052: the reserved workers hand an arbitrary request word (the
    // second-stage record); when set, the call answers with this pointer
    // instead of a fresh malloc (the binary's session object allocation is
    // opaque here).
    void* forced_result = nullptr;
    int calls = 0;
    // Lot 052: the 18-dword error-report descriptor lives on the live
    // worker stack; copy the words during the call, because the frame is
    // dead (and overwritten by the caller's own call machinery) as soon
    // as the op returns. The length word identifies the call.
    bool want_save = false;
    std::int32_t save_length = -1;
    const void* saved_request = nullptr;
    std::uint32_t saved_words[18] = {};
    virtual void* allocate(std::uint32_t length, void* request) {
        got_length = length;
        got_request = request;
        ++calls;
        if (want_save &&
            static_cast<std::int32_t>(length) == save_length &&
            request != nullptr) {
            saved_request = request;
            std::memcpy(saved_words, request, sizeof saved_words);
            want_save = false;
        }
        if (forced_result != nullptr) return forced_result;
        return std::malloc(length);
    }
};
PathAllocator allocator;

// Completion stub of the create-and-wait wrappers: vslot 0 = release pair
// (built by the compiler vftable of DoneNodeMethods) and vslot 2 = the
// 64-bit result pair read by 006b2359's call shape at the moment the node
// reaches the completed list.
std::uint64_t done_pair_value = 0;
std::uint8_t* drain_node = nullptr;

struct DoneNodeMethods {
    virtual void release(int) {}
    virtual void unused() {}
    virtual std::uint64_t read_pair() { return done_pair_value; }
};

// 006b2553 callback probe (capture-less, so it reads the reserve word
// through a global pointer installed by the harness).
std::uint8_t* reserve_probe = nullptr;
std::uint32_t reserve_seen = 0;
std::uint32_t reserved_callback(std::uint32_t value, std::int32_t extra) {
    (void)extra;
    reserve_seen = *reinterpret_cast<std::uint32_t*>(reserve_probe);
    return value;
}

// Scheduled on the same wheel: completes the node being waited for while
// the owning thread pumps (detach from the frame's pending list, install
// the stub vtable + result code, park in the completed list). Lot 052:
// with `drain_node` empty the sweep completes the head of whichever
// alive frame's pending list is non-empty, which drives the multi-stage
// reserved file-op chains; the always-due wheel registration
// {callback, id, period 0, due 0} keeps it armed between pumps.
void fire_drain(std::int32_t argument, std::int32_t overdue) {
    (void)argument;
    (void)overdue;
    auto* root = reinterpret_cast<std::uint8_t*>(DAT_00892b60);
    auto* target = drain_node;
    if (target == nullptr) {
        const auto* bases = reinterpret_cast<std::uint32_t*>(root + 0x98);
        for (std::uint32_t index = 0; index < 8 && target == nullptr; ++index) {
            auto* frame = reinterpret_cast<std::uint8_t*>(*bases + index * 0x88);
            target = reinterpret_cast<std::uint8_t*>(
                *reinterpret_cast<std::uint32_t*>(frame + 0x10));
        }
    }
    if (target == nullptr) return;
    const std::uint32_t token = *reinterpret_cast<std::uint32_t*>(target + 0xc);
    auto* frame = reinterpret_cast<std::uint8_t*>(
        (*reinterpret_cast<std::uint32_t*>(root + 0x98)) +
        (token & 0xff) * 0x88);
    auto* list = frame + 0x10;
    std::uint8_t* previous = nullptr;
    std::uint8_t* current = reinterpret_cast<std::uint8_t*>(*reinterpret_cast<std::uint32_t*>(list));
    while (current != nullptr && current != target) {
        previous = current;
        const std::uint32_t link =
            *reinterpret_cast<std::uint32_t*>(current + 8);
        current = link == 0 ? nullptr : reinterpret_cast<std::uint8_t*>(link - 8);
    }
    if (current == nullptr) return;
    --*reinterpret_cast<std::uint32_t*>(list + 8);
    if (previous == nullptr) {
        const std::uint32_t link =
            *reinterpret_cast<std::uint32_t*>(current + 8);
        *reinterpret_cast<std::uint32_t*>(list) = link == 0 ? 0u : link - 8u;
        if (*reinterpret_cast<std::uint32_t*>(list + 4) == address(current)) {
            *reinterpret_cast<std::uint32_t*>(list + 4) = 0;
        }
    } else {
        *reinterpret_cast<std::uint32_t*>(previous + 8) =
            *reinterpret_cast<std::uint32_t*>(current + 8);
        if (*reinterpret_cast<std::uint32_t*>(current + 8) == 0) {
            *reinterpret_cast<std::uint32_t*>(list + 4) = address(previous);
        }
    }
    *reinterpret_cast<std::uint32_t*>(target + 8) = 0;
    DoneNodeMethods methods;
    *reinterpret_cast<void**>(target) = *reinterpret_cast<void**>(&methods);
    *reinterpret_cast<std::uint8_t*>(target + 0x11) = 7;
    hp6::runtime::insert_after(frame + 0x1c, nullptr, target);
    if (drain_node != nullptr) {
        drain_node = nullptr;
        // One-shot in explicit-target mode: the wheel slot clears itself.
        hp6::platform::timer_slots_00899ad8[5] = {};
    }
}

// Volume object for the lot-047 creators: vtable slot +0x30 (index 12)
// answers with the record's own static name and ignores the optional
// buffer argument (as established for the +0x30 slot).
struct PoolVolumeNode {
    virtual void v0() {}
    virtual void v1() {}
    virtual void v2() {}
    virtual void v3() {}
    virtual void v4() {}
    virtual void v5() {}
    virtual void v6() {}
    virtual void v7() {}
    virtual void v8() {}
    virtual void v9() {}
    virtual void va() {}
    virtual void vb() {}
    const char* name_text;
    explicit PoolVolumeNode(const char* text) : name_text(text) {}
    virtual char* name(char* /*buffer*/) {
        return const_cast<char*>(name_text);
    }
};
PoolVolumeNode volume_c("C:");

} // namespace

int main() {
    using namespace hp6::platform;
    using namespace hp6::runtime;
    static_assert(sizeof(void*) == 4, "The binary stores 32-bit addresses");
    static_assert(sizeof(TimerSlot) == 0x14, "Wheel stride of 20 bytes");

    DAT_00899acc = GetCurrentThreadId();
    DAT_00899c1c = -1;

    // ---- pump_timer_slots (006d971d) -----------------------------------
    auto* wheel = timer_slots_00899ad8;
    std::memset(wheel, 0, 16 * sizeof(TimerSlot));
    DAT_00899d88 = 1000;
    wheel[0] = {&fire, 77, 100, 900, 0};          // overdue: fires now
    wheel[1] = {&fire, 78, 100, 900, 1};          // busy: skipped
    wheel[2] = {nullptr, 79, 100, 0, 0};          // no callback: skipped
    wheel[3] = {&fire, 80, 200, 5000, 0};         // future due: skipped
    pump_timer_slots();
    expect("pump_fires", fires == 1);
    expect("pump_args", fire_arg == 77 && fire_overdue == 100);
    expect("pump_reschedule", wheel[0].due == 1100);
    expect("pump_busy_clear", wheel[0].busy == 0);
    expect("pump_busy_kept", wheel[1].busy == 1);
    expect("pump_snapshot", DAT_00899c1c == 1000);
    expect("pump_future_skip", wheel[3].due == 5000);

    pump_timer_slots();                            // nothing due again
    expect("pump_second_pass", fires == 1 && wheel[0].due == 1100);

    // Not the owning thread: the wheel is untouched.
    DAT_00899acc ^= 1u;
    wheel[4] = {&fire, 81, 100, 0, 0};
    pump_timer_slots();
    expect("pump_foreign", fires == 1 && wheel[4].due == 0 &&
                             DAT_00899c1c == 1000);
    DAT_00899acc ^= 1u;

    // ---- wait_pool_node (006b22c3) -------------------------------------
    std::memset(root, 0, sizeof root);
    std::memset(slots, 0, sizeof slots);
    DAT_00892b60 = static_cast<std::int32_t>(address(root));
    word(root, 0x98) = address(slots);
    word(root, 0x50) = 5;                          // slot-token counter
    initialize_lock(reinterpret_cast<std::uintptr_t>(root + 0xc));
    initialize_lock(reinterpret_cast<std::uintptr_t>(root + 0x54));

    expect("wait_zero_token", wait_pool_node(0) == -3);

    auto* frame3 = slots + 3 * 0x88;
    initialize_lock(reinterpret_cast<std::uintptr_t>(frame3 + 0x28));
    word(frame3, 4) = 1;                           // slot alive
    word(frame3, 0x48) = reinterpret_cast<std::uint32_t>(
        CreateEventA(nullptr, FALSE, FALSE, nullptr));
    const std::uint32_t token3 = (5u << 8) | 3u;
    // Not found on the first search: the flag clears and 006b2223 answers -3.
    expect("wait_not_found", wait_pool_node(token3) == -3);

    // ---- create_node_from_record (006b2456) ----------------------------
    alignas(16) std::uint8_t node[0x40];
    alignas(16) std::uint8_t record[0x20];
    std::memset(node, 0, sizeof node);
    std::memset(record, 0, sizeof record);
    insert_first(root, node);
    word(record, 0xc) = address(frame3);
    const std::uint32_t token = create_node_from_record(
        reinterpret_cast<std::uintptr_t>(record), 7, 0x99);
    expect("create_token", token == token3);
    expect("create_id", word(node, 0x14) == address(record));
    expect("create_vtable",
           *reinterpret_cast<void**>(node) ==
               static_cast<void*>(&PTR_FUN_00715ac0));
    // 006b2199 queued the node into the engine list at frame3 + 0x10 and
    // signaled the auto-reset event at +0x48.
    expect("create_scheduled", word(frame3, 0x10 + 8) == 1 &&
                                   word(frame3, 0x10) == address(node));
    expect("create_signaled",
           WaitForSingleObject(reinterpret_cast<HANDLE>(
                                   static_cast<std::uintptr_t>(
                                       word(frame3, 0x48))),
                               0) == WAIT_OBJECT_0);

    // The binary has no empty-pool guard: with an empty pool this call
    // would fault at [null+0xc] exactly like 006b2456 does (verified in
    // the exported x86), so only the populated path is exercised here.
    alignas(16) std::uint8_t node3b[0x40];
    std::memset(node3b, 0, sizeof node3b);
    insert_first(root, node3b);

    // ---- create_repeat_node_from_record (006b24b7) ---------------------
    alignas(16) std::uint8_t node2[0x50];
    std::memset(node2, 0, sizeof node2);
    insert_first(root, node2);
    const std::uint32_t token2 = create_repeat_node_from_record(
        reinterpret_cast<std::uintptr_t>(record), 0x21, 0x22, 0x23, 0x24,
        0x25, 9, 0x1234);
    const std::uint32_t token3b = (6u << 8) | 3u;   // counter advanced
    expect("repeat_token", token2 == token3b);
    expect("repeat_id", word(node2, 0x14) == address(record));
    expect("repeat_vtable",
           *reinterpret_cast<void**>(node2) ==
               static_cast<void*>(&PTR_FUN_00715ae0));
    expect("repeat_fields",
           word(node2, 0x28) == 0x21 && word(node2, 0x2c) == 0x22 &&
               word(node2, 0x20) == 0x23 && word(node2, 0x24) == 0x24 &&
               word(node2, 0x30) == 0x25 && word(node2, 0x18) == 0x1234);
    expect("repeat_cleared", word(node2, 0x38) == 0 && word(node2, 0x3c) == 0);
    expect("repeat_mode", node2[0x12] == 9);

    // ---- construct_signed_node (006b3094) / construct_plain_node (006b335c)
    alignas(16) std::uint8_t node3[0x50];
    alignas(16) std::uint8_t node4[0x50];
    std::memset(node3, 0, sizeof node3);
    std::memset(node4, 0, sizeof node4);
    construct_signed_node(node3, 77, -5, 9, 3, 2, 0x4567,
                          reinterpret_cast<std::uintptr_t>(frame3));
    expect("signed_id", word(node3, 0x14) == 77);
    expect("signed_bounds",
           static_cast<std::int32_t>(word(node3, 0x28)) == -5 &&
               static_cast<std::int32_t>(word(node3, 0x2c)) == -1 &&
               static_cast<std::int32_t>(word(node3, 0x20)) == 9 &&
               word(node3, 0x24) == 0);
    expect("signed_rest", word(node3, 0x30) == 3 && word(node3, 0x18) == 0x4567 &&
                              node3[0x12] == 2 && word(node3, 0x38) == 0 &&
                              word(node3, 0x3c) == 0);
    expect("signed_vtable",
           *reinterpret_cast<void**>(node3) == static_cast<void*>(&PTR_FUN_00715ad0));

    construct_plain_node(node4, 88, 3, 0x2345,
                         reinterpret_cast<std::uintptr_t>(frame3));
    expect("plain_vtable",
           *reinterpret_cast<void**>(node4) == static_cast<void*>(&PTR_FUN_00715b00));
    expect("plain_fields", word(node4, 0x14) == 88 && word(node4, 0x18) == 0x2345 &&
                               node4[0x12] == 3);
    expect("plain_scheduled", word(frame3, 0x10 + 8) == 4);

    // Second populated creation (006b2456); the four explicit attaches
    // above left the counter at 9, so the token carries (9<<8)|3.
    const std::uint32_t again_token = create_node_from_record(
        reinterpret_cast<std::uintptr_t>(record), 7, 0x99);
    expect("create_again", again_token == ((9u << 8) | 3u));

    // ---- attach_node_path (006b1968) -----------------------------------
    // The record block handed to slot +0 carries the filesys.cpp loader
    // error context (type 6, line 0x1cc, sub-record id 5, length word),
    // and the allocation covers strlen + 1.
    DAT_00851384 = &allocator;
    alignas(16) std::uint8_t node5[0x50];
    std::memset(node5, 0, sizeof node5);
    {
        const char* path5 = "D:\\maps\\a.bsp";
        attach_node_path(node5, path5);
        void* stored = *reinterpret_cast<void**>(node5 + 0x30);
        expect("path_stored", std::strcmp(static_cast<const char*>(stored),
                                          path5) == 0);
        expect("path_alloc_len",
               allocator.got_length == static_cast<std::uint32_t>(
                                           std::strlen(path5) + 1));
        const auto* req = static_cast<const std::uint32_t*>(
            const_cast<const void*>(allocator.got_request));
        expect("path_record",
               req[0] == 6 && req[1] == 0x1cc && req[3] == 5 && req[9] == 4 &&
                   req[10] == 1 && req[11] == 0 &&
                   req[4] == address(&node_source_path_00715a30) &&
                   req[7] == address(path5) &&
                   req[2] == address(&req[3]) && req[5] == address(&req[6]) &&
                   req[8] == address(&req[9]));
        std::free(stored);
    }
    // Null path falls back to the empty string literal at 0x007278d3 and
    // copies just the terminating NUL.
    attach_node_path(node5, nullptr);
    {
        void* stored = *reinterpret_cast<void**>(node5 + 0x30);
        expect("path_null", *static_cast<const char*>(stored) == '\0');
        const auto* req = static_cast<const std::uint32_t*>(
            const_cast<const void*>(allocator.got_request));
        expect("path_null_len", allocator.got_length == 1 &&
                                    req[7] == address(&empty_string_007278d3));
        std::free(stored);
    }

    // ---- named constructors (006b2ef3 / 006b2f40) -----------------------
    alignas(16) std::uint8_t node6[0x50];
    std::memset(node6, 0, sizeof node6);
    construct_named_node_00715a90(node6, "D:\\gfx\\x.tex", 4, 0x3333,
                                  reinterpret_cast<std::uintptr_t>(frame3));
    expect("named_a_vtable",
           *reinterpret_cast<void**>(node6) == static_cast<void*>(&PTR_FUN_00715a90));
    expect("named_a_path", std::strcmp(static_cast<const char*>(
                                           *reinterpret_cast<void**>(node6 + 0x30)),
                                       "D:\\gfx\\x.tex") == 0);
    expect("named_a_rest", node6[0x12] == 4 && word(node6, 0x18) == 0x3333);
    std::free(*reinterpret_cast<void**>(node6 + 0x30));

    alignas(16) std::uint8_t node7[0x50];
    std::memset(node7, 0, sizeof node7);
    construct_named_node_00715aa0(node7, "D:\\gfx\\y.tex", 5, 0x4444,
                                  reinterpret_cast<std::uintptr_t>(frame3));
    expect("named_b_vtable",
           *reinterpret_cast<void**>(node7) == static_cast<void*>(&PTR_FUN_00715aa0));
    expect("named_b_rest", node7[0x12] == 5 && word(node7, 0x18) == 0x4444);

    // ---- record-fed signed / plain creators (006b2482 / 006b2527) -------
    alignas(16) std::uint8_t slot5[0x40];
    std::memset(slot5, 0, sizeof slot5);
    insert_first(root, slot5);
    // The two named constructions above attached their receivers, so the
    // counter reached 12 before the signed creation.
    expect("create_signed", create_signed_node_from_record(
                                reinterpret_cast<std::uintptr_t>(record), 9, 3,
                                -5, 2, 0x4567) == ((12u << 8) | 3u));
    expect("signed_creator_fields",
           word(slot5, 0x14) == address(record) &&
               static_cast<std::int32_t>(word(slot5, 0x28)) == -5 &&
               static_cast<std::int32_t>(word(slot5, 0x2c)) == -1 &&
               static_cast<std::int32_t>(word(slot5, 0x20)) == 9 &&
               word(slot5, 0x24) == 0 && word(slot5, 0x30) == 3 &&
               slot5[0x12] == 2 && word(slot5, 0x18) == 0x4567);

    alignas(16) std::uint8_t slot6[0x40];
    std::memset(slot6, 0, sizeof slot6);
    insert_first(root, slot6);
    expect("create_plain_from_record",
           create_plain_node_from_record(reinterpret_cast<std::uintptr_t>(record),
                                         6, 0x2222) == ((13u << 8) | 3u));
    expect("plain_creator_fields", word(slot6, 0x14) == address(record) &&
                                       word(slot6, 0x18) == 0x2222 &&
                                       slot6[0x12] == 6);

    // ---- lot 047: bounded path constructor and volume-aware creators ---
    // A aligned volume record so find_volume_by_name scans it, engine
    // index 4 at slots + 4*0x88.
    static char game_path[] = "C:\\bin\\game.exe";
    static char* game_path_slot[1];
    game_path_slot[0] = game_path;
    word(root, 0x88) = address(game_path_slot);
    std::uint8_t* record4 = slots + 4 * 0x88;
    word(root, 0x7c) = address(record4);
    word(record4, 0x84) = address(&volume_c);
    word(record4, 4) = 1;                          // slot alive (skip bootstrap)
    // The scheduler touches the frame lock at +0x28 (CS at +4).
    initialize_lock(reinterpret_cast<std::uintptr_t>(record4 + 0x28));
    word(record4, 0x48) = reinterpret_cast<std::uint32_t>(
        CreateEventA(nullptr, FALSE, FALSE, nullptr));

    alignas(16) std::uint8_t node8[0x50];
    std::memset(node8, 0, sizeof node8);
    const auto token_bounded = (word(root, 0x50) << 8) | 4u;
    construct_bounded_path_node(node8, "C:\\run2", 0x42, 8, 0xCCCC,
                                reinterpret_cast<std::uintptr_t>(record4));
    expect("bounded_vtable",
           *reinterpret_cast<void**>(node8) == static_cast<void*>(&PTR_FUN_00715ab0));
    expect("bounded_bounds", word(node8, 0x20) == 0x42 && word(node8, 0x24) == 0);
    expect("bounded_path",
           std::strcmp(static_cast<const char*>(
                           *reinterpret_cast<void**>(node8 + 0x30)),
                       "C:\\run2") == 0 &&
               node8[0x12] == 8 && word(node8, 0x18) == 0xCCCC);

    // 006b289e: engine from the path itself ("C:" matches directly).
    alignas(16) std::uint8_t slot7[0x40];    std::memset(slot7, 0, sizeof slot7);
    insert_first(root, slot7);
    const auto token_a = (word(root, 0x50) << 8) | 4u;
    expect("create_named_a", create_named_node_a_from_record(
                                 "C:\\levels\\one", 3, 0x8888) ==
                                 token_a);
    expect("named_a_path",
           std::strcmp(static_cast<const char*>(
                           *reinterpret_cast<void**>(slot7 + 0x30)),
                       "C:\\levels\\one") == 0);

    // 006b28d4: relative path - volume name "C:" + path.
    alignas(16) std::uint8_t slot8[0x40];
    std::memset(slot8, 0, sizeof slot8);
    insert_first(root, slot8);
    const auto token_b1 = (word(root, 0x50) << 8) | 4u;
    expect("create_named_b_relative",
           create_named_node_b_from_volume("\\maps\\a", 5, 0x9999) ==
               token_b1);
    expect("named_b_full",
           std::strcmp(static_cast<const char*>(
                           *reinterpret_cast<void**>(slot8 + 0x30)),
                       "C:\\maps\\a") == 0 &&
               node8[0x12] == 8);

    // UNC path: no name consultation, path unchanged.
    alignas(16) std::uint8_t slot9[0x40];
    std::memset(slot9, 0, sizeof slot9);
    insert_first(root, slot9);
    const auto token_b2 = (word(root, 0x50) << 8) | 4u;
    expect("create_named_b_unc",
           create_named_node_b_from_volume("\\\\srv\\x", 6, 0xAAAA) ==
               token_b2);
    expect("named_b_unc_path",
           std::strcmp(static_cast<const char*>(
                           *reinterpret_cast<void**>(slot9 + 0x30)),
                       "\\\\srv\\x") == 0);

    // 006b2990: bounded path node fed by the record path.
    alignas(16) std::uint8_t slot10[0x40];
    std::memset(slot10, 0, sizeof slot10);
    insert_first(root, slot10);
    const auto token_bp = (word(root, 0x50) << 8) | 4u;
    expect("create_bounded_path",
           create_bounded_path_node_from_record("C:\\run3", 0x43, 9,
                                                0xDDDD) == token_bp);
    expect("bounded_path_fields",
           word(slot10, 0x20) == 0x43 && word(slot10, 0x24) == 0 &&
               word(slot10, 0x18) == 0xDDDD && slot10[0x12] == 9);

    // ---- 006db315 / 006db335: intrusive FIFO push/pop -------------------
    alignas(16) std::uint8_t queue[0x20];
    std::memset(queue, 0, sizeof queue);
    expect("queue_pop_empty", word(queue, 8) == 0);
    queue_pop_006db335(queue);                       // no-op, count intact
    expect("queue_pop_empty_count", word(queue, 8) == 0);

    std::uint32_t link_a = 0;
    std::uint32_t link_b = 0;
    std::uint32_t link_c = 0;
    queue_push_006db315(queue, reinterpret_cast<std::uint8_t*>(&link_a));
    queue_push_006db315(queue, reinterpret_cast<std::uint8_t*>(&link_b));
    expect("queue_pushed", word(queue, 8) == 2 && word(queue, 0) == address(&link_a) &&
                               word(queue, 4) == address(&link_b) &&
                               link_a == address(&link_b) && link_b == 0);
    queue_pop_006db335(queue);
    expect("queue_popped_a", word(queue, 0) == address(&link_b) &&
                                 word(queue, 4) == address(&link_b) &&
                                 word(queue, 8) == 1 && link_a == 0);
    queue_push_006db315(queue, reinterpret_cast<std::uint8_t*>(&link_c));
    expect("queue_pushed_c", word(queue, 8) == 2 && link_b == address(&link_c) &&
                                 word(queue, 4) == address(&link_c));
    queue_pop_006db335(queue);
    queue_pop_006db335(queue);
    expect("queue_drained", word(queue, 0) == 0 && word(queue, 4) == 0 &&
                                word(queue, 8) == 0 && link_b == 0 &&
                                link_c == 0);
    queue_pop_006db335(queue);
    expect("queue_exhausted", word(queue, 0) == 0 && word(queue, 8) == 0);

    // ---- wrapper no-op stubs (006b454a/55/60, 006b54bb/be) --------------
    // Their bodies write nothing; asserts only confirm no observable side
    // effects through the queue header and count.
    alignas(16) std::uint8_t probe[0x10];
    std::memset(probe, 0x5a, sizeof probe);
    wrap_noop_006b454a(probe, 1);
    wrap_noop_006b4555(probe, 2);
    wrap_noop_006b4560(probe, 3, 4);
    wrap_noop_006b54bb(probe, 5);
    wrap_noop_006b54be(probe);
    for (int i = 0; i < 16; ++i) {
        if (probe[i] != 0x5a) ++failures;
    }
    expect("wrap_noop_sterile", failures == 0);

    // ---- lot 049: create-and-wait wrappers and the reserved callback ----
    // The owning thread pumps the wheel inside the wait, so the one-shot
    // drain slot completes the freshly created node.
    DAT_00899acc = GetCurrentThreadId();
    done_pair_value = 0x1234567800000000ull;

    alignas(16) std::uint8_t slot11[0x40];
    std::memset(slot11, 0, sizeof slot11);
    insert_first(root, slot11);
    drain_node = slot11;
    wheel[5] = {&fire_drain, 91, 100, 900, 0};
    const auto pending_before11 = word(frame3, 0x10 + 8);
    expect("wait_node", create_and_wait_node(
                            reinterpret_cast<std::uintptr_t>(record), 7));
    expect("wait_node_done", word(frame3, 0x10 + 8) == pending_before11 &&
                                 slot11[0x11] == 7);
    word(frame3, 0x1c) = 0;
    word(frame3, 0x1c + 8) = 0;
    alignas(16) std::uint8_t slot12[0x40];
    std::memset(slot12, 0, sizeof slot12);
    insert_first(root, slot12);
    drain_node = slot12;
    wheel[5] = {&fire_drain, 92, 100, 900, 0};
    const auto pending_before12 = word(frame3, 0x10 + 8);
    expect("wait_repeat_pair",
           create_and_wait_repeat_node(reinterpret_cast<std::uintptr_t>(record),
                                       -3, 0x555, -9, 8) ==
               static_cast<std::int64_t>(done_pair_value));
    expect("wait_repeat_fields", word(slot12, 0x14) == address(record) &&
                                     static_cast<std::int32_t>(word(slot12, 0x28)) == -3 &&
                                     static_cast<std::int32_t>(word(slot12, 0x2c)) == -1 &&
                                     word(slot12, 0x20) == 0x555 &&
                                     static_cast<std::int32_t>(word(slot12, 0x24)) == -9 &&
                                     static_cast<std::int32_t>(word(slot12, 0x30)) == -1 &&
                                     slot12[0x12] == 8);
    word(frame3, 0x1c) = 0;
    word(frame3, 0x1c + 8) = 0;

    alignas(16) std::uint8_t slot13[0x40];
    std::memset(slot13, 0, sizeof slot13);
    insert_first(root, slot13);
    drain_node = slot13;
    wheel[5] = {&fire_drain, 93, 100, 900, 0};
    expect("wait_bounded_path_pair",
           create_and_wait_bounded_path_node("C:\\run4", 0x44, 9) ==
               static_cast<std::int64_t>(done_pair_value));
    expect("wait_bounded_fields", word(slot13, 0x20) == 0x44 &&
                                      word(slot13, 0x24) == 0 &&
                                      slot13[0x12] == 9);
    word(record4, 0x1c) = 0;
    word(record4, 0x1c + 8) = 0;

    alignas(16) std::uint8_t slot14[0x40];
    std::memset(slot14, 0, sizeof slot14);
    insert_first(root, slot14);
    drain_node = slot14;
    wheel[5] = {&fire_drain, 94, 100, 900, 0};
    expect("wait_named_node", create_and_wait_named_node("C:\\run5", 6));
    expect("wait_named_path",
           std::strcmp(static_cast<const char*>(*reinterpret_cast<void**>(slot14 + 0x30)),
                       "C:\\run5") == 0);
    word(record4, 0x1c) = 0;
    word(record4, 0x1c + 8) = 0;

    // 006b23a9: passthrough straight onto a synthetic completed node.
    alignas(16) std::uint8_t slot15[0x40];
    std::memset(slot15, 0, sizeof slot15);
    word(slot15, 0xc) = (word(root, 0x50) << 8) | 3u;
    DoneNodeMethods methods;
    *reinterpret_cast<void**>(slot15) = *reinterpret_cast<void**>(&methods);
    slot15[0x11] = 3;
    insert_after(frame3 + 0x1c, nullptr, slot15);
    expect("completed_state", completed_state(word(slot15, 0xc)) ==
                                  static_cast<std::int64_t>(done_pair_value));

    // 006b2553: reserved callback. Branch 1: the record carries its own
    // sync frame (slot flag +0x80 zero); branch 2: the wheel base carries
    // it. The callback probes the reserve word through a global pointer
    // (capture-less lambdas cannot close over locals).
    reserve_probe = record4 + 0x7c;
    initialize_lock(reinterpret_cast<std::uintptr_t>(record4 + 0x4c));
    word(record4, 0x7c) = 0xbcd;
    const auto cb_return = run_reserved_callback(
        reserved_callback,
        reinterpret_cast<std::uintptr_t>(record4), 0x1234, -7);
    expect("reserved_rec", cb_return == 0x1234u && reserve_seen == 0x1234u &&
                               word(record4, 0x7c) == 0xbcd);

    // Branch 2: the flag at +0x80 routes the lock to the wheel base.
    word(record4, 0x80) = 1;
    initialize_lock(reinterpret_cast<std::uintptr_t>(slots + 0x4c));
    word(slots, 0x48) = reinterpret_cast<std::uint32_t>(
        CreateEventA(nullptr, FALSE, FALSE, nullptr));
    reserve_probe = slots + 0x7c;
    const auto cb_return2 = run_reserved_callback(
        reserved_callback, reinterpret_cast<std::uintptr_t>(record4),
        0x4567, 12);
    expect("reserved_base", cb_return2 == 0x4567u && reserve_seen == 0x4567u);
    word(record4, 0x80) = 0;

    // ---- lot 051: plain create-and-wait ---------------------------------
    alignas(16) std::uint8_t slot16[0x40];
    std::memset(slot16, 0, sizeof slot16);
    insert_first(root, slot16);
    drain_node = slot16;
    wheel[5] = {&fire_drain, 95, 100, 900, 0};
    expect("wait_plain_pair",
           create_and_wait_plain_node(reinterpret_cast<std::uintptr_t>(record),
                                      4) ==
               static_cast<std::int64_t>(done_pair_value));
    expect("wait_plain_fields", word(slot16, 0x14) == address(record) &&
                                    word(slot16, 0x18) == 0 &&
                                    slot16[0x12] == 4);
    word(frame3, 0x1c) = 0;
    word(frame3, 0x1c + 8) = 0;

    // ---- 006b2dde: FIFO-specific removal on the ListQueue shape ---------
    alignas(16) std::uint8_t queue2[0x20];
    std::memset(queue2, 0, sizeof queue2);
    std::uint32_t q2[4] = {};
    for (int i = 0; i < 4; ++i) {
        queue_push_006db315(queue2,
                            reinterpret_cast<std::uint8_t*>(&q2[i]));
    }
    expect("queue_remove_mid",
           queue_remove_006b2dde(queue2,
                                 reinterpret_cast<std::uint8_t*>(&q2[1]),
                                 nullptr) == 1 &&
               word(queue2, 8) == 3 && q2[0] == address(&q2[2]) &&
               word(queue2, 4) == address(&q2[3]) && q2[1] == 0);
    expect("queue_remove_head",
           queue_remove_006b2dde(queue2,
                                 reinterpret_cast<std::uint8_t*>(&q2[0]),
                                 nullptr) == 1 &&
               word(queue2, 0) == address(&q2[2]) && word(queue2, 8) == 2);
    expect("queue_remove_tail",
           queue_remove_006b2dde(queue2,
                                 reinterpret_cast<std::uint8_t*>(&q2[3]),
                                 nullptr) == 1 &&
               word(queue2, 4) == address(&q2[2]) && word(queue2, 8) == 1 &&
               q2[3] == 0);
    std::uint32_t q2_missing = 0;
    expect("queue_remove_missing",
           queue_remove_006b2dde(queue2,
                                 reinterpret_cast<std::uint8_t*>(&q2_missing),
                                 nullptr) == 0 &&
               word(queue2, 8) == 1 && word(queue2, 0) == address(&q2[2]));

    // ---- 006b45ae: wrapper state reset ----------------------------------
    alignas(16) std::uint8_t wblock[0x18];
    std::memset(wblock, 0x77, sizeof wblock);
    reset_wrapper_block_006b45ae(wblock);
    expect("wrapper_reset",
           *reinterpret_cast<std::uint32_t*>(wblock) == 0xffffffffu &&
               *reinterpret_cast<std::uint32_t*>(wblock + 4) == 0xffffffffu &&
               *reinterpret_cast<std::uint32_t*>(wblock + 8) == 0 &&
               *reinterpret_cast<std::uint32_t*>(wblock + 0xc) == 0 &&
               *reinterpret_cast<std::uint32_t*>(wblock + 0x10) == 0 &&
               wblock[0x14] == 0 && wblock[0x15] == 0 && wblock[0x16] == 0x77);

    // ---- 006b1a59: ';' search path splitter ------------------------------
    // Runs after every volume/path consumer so the restored array keeps
    // its lot-047 setup.
    DAT_00851390 = 4;
    DAT_00851394 = 0x40;
    alignas(16) static char path_buffer1a59[0x40];
    static char* path_slots_1a59[4];
    word(root, 0x88) = address(path_slots_1a59);
    word(root, 0x8c) = address(path_buffer1a59);
    std::memset(path_buffer1a59, 0xcc, sizeof path_buffer1a59);
    set_game_search_paths("C:\\bin\\;C:\\data;models\\");
    expect("search_path_head",
           path_slots_1a59[0] == path_buffer1a59 &&
               std::strcmp(path_slots_1a59[0], "C:\\bin") == 0);
    expect("search_path_tok1",
           std::strcmp(path_slots_1a59[1], "C:\\data") == 0);
    expect("search_path_tok2",
           std::strcmp(path_slots_1a59[2], "models") == 0);
    expect("search_path_sterile", path_buffer1a59[0x30] == 0);
    word(root, 0x88) = address(game_path_slot);

    // ---- lot 052: reserved file operators (mixed_deps_060) ---------------
    // The wrappers park a 0x64-reserved context on their stack and hand it
    // to the reserved callback; the workers chain the bounded-path record
    // (pair low word) through plain/repeat/probe stages whose completion
    // pairs all read `done_pair_value`, so the pair-low is a real record
    // the following stages deref at +0xc (the engine word).
    DAT_00899acc = GetCurrentThreadId();
    DAT_008513ac = 77;
    alignas(16) std::uint8_t fake_file_record[0x20];
    std::memset(fake_file_record, 0, sizeof fake_file_record);
    word(fake_file_record, 0xc) = address(frame3);
    const std::uint32_t fake_record_addr = address(fake_file_record);
    done_pair_value = static_cast<std::uint64_t>(fake_record_addr);

    // Fresh pending/done lists for the two engine frames: the sweep then
    // only dislodges freshly created nodes.
    for (std::uint32_t offset = 0; offset <= 0xc; offset += 4) {
        word(frame3, 0x10 + offset) = 0;
        word(frame3, 0x1c + offset) = 0;
        word(record4, 0x10 + offset) = 0;
        word(record4, 0x1c + offset) = 0;
    }

    // Thirteen fresh 0x40 slots: open chain 4 stages, write chain 3,
    // probe chain 3, probe-miss chain 3. Detach pops the last inserted
    // first, so stage 1 of the first op takes fops[12], and so on.
    alignas(16) std::uint8_t fops[13][0x40];
    for (auto& slot : fops) {
        std::memset(slot, 0, sizeof slot);
        insert_first(root, slot);
    }

    // Always-due sweep registration (period 0): re-arms after every pump
    // and drives each stage completion of the chains below.
    wheel[5] = {&fire_drain, 96, 0, 0, 0};

    const auto calls_before_fops = allocator.calls;
    // A real mapped buffer: the same allocator object also serves the
    // path attachments inside the chains (their memcpy targets it).
    alignas(16) static std::uint8_t forced_buffer[0x40];
    std::memset(forced_buffer, 0, sizeof forced_buffer);
    allocator.forced_result = forced_buffer;
    static const char open_path[] = "C:\\fop1";
    std::uint32_t open_out = 0;
    // The error-report call is identified by its length (the second-stage
    // record): the allocator copies the descriptor while the worker's
    // stack is still alive.
    allocator.save_length = static_cast<std::int32_t>(fake_record_addr);
    allocator.want_save = true;
    open_file_from_path(open_path, &open_out, 0x51);
    expect("fops_open_stage1",
           word(fops[12], 0x20) == 1 && fops[12][0x12] == 100);
    expect("fops_open_stage2",
           word(fops[11], 0x14) == fake_record_addr && fops[11][0x12] == 99);
    expect("fops_open_repeat",
           word(fops[10], 0x14) == fake_record_addr &&
               word(fops[10], 0x20) == address(forced_buffer) &&
               word(fops[10], 0x28) == 0 &&
               word(fops[10], 0x24) == fake_record_addr &&
               fops[10][0x12] == 99);
    expect("fops_open_out", open_out == fake_record_addr);
    // Two allocations in the open chain: the stage-1 path attach, then
    // the error-report call whose request is the descriptor itself and
    // whose length is the second-stage record.
    expect("fops_open_alloc",
           allocator.calls == calls_before_fops + 2 &&
               allocator.got_length == fake_record_addr &&
               allocator.got_request != nullptr);
    {
        const auto* req = allocator.saved_words;
        const auto descriptor_base =
            reinterpret_cast<std::uintptr_t>(allocator.saved_request);
        expect("fops_open_record",
               req[0] == 6 && req[1] == 0xac && req[3] == 5 && req[6] == 7 &&
                   req[9] == 1 && req[16] == 1 && req[17] == 0 &&
                   req[10] == address(open_path) &&
                   req[4] == address(&hlsfile_source_0071b358) &&
                   req[13] == 77u &&
                   req[2] == descriptor_base + 12 &&
                   req[5] == descriptor_base + 24 &&
                   req[8] == descriptor_base + 36 &&
                   req[11] == descriptor_base + 48);
        expect("fops_open_record_ptr", req[14] == descriptor_base + 60);
    }

    static const char write_path[] = "C:\\fop2";
    write_file_flagged(write_path, 0x15, 0x16, 0x17);
    expect("fops_write_stage1",
           word(fops[8], 0x20) == 1 && fops[8][0x12] == 100);
    expect("fops_write_repeat",
           word(fops[7], 0x14) == fake_record_addr &&
               word(fops[7], 0x20) == 0x15 &&
               static_cast<std::int32_t>(word(fops[7], 0x24)) == 0x16 &&
               word(fops[7], 0x28) == 0 && word(fops[7], 0x2c) == 0 &&
               word(fops[7], 0x30) == 0 && fops[7][0x12] == 99);
    expect("fops_write_plain",
           word(fops[6], 0x14) == fake_record_addr && fops[6][0x12] == 99);
    // The write chain allocates once: its stage-1 path attach.
    expect("fops_write_alloc", allocator.calls == calls_before_fops + 3);

    static const char probe_path[] = "C:\\fop3";
    expect("fops_check_hit",
           check_file_flagged(probe_path, 0x25, fake_record_addr) == true);
    expect("fops_check_stage1",
           word(fops[5], 0x20) == 6 && fops[5][0x12] == 100);
    expect("fops_check_probe",
           word(fops[4], 0x14) == fake_record_addr && fops[4][0x12] == 99 &&
               word(fops[4], 0x20) == 0 && word(fops[4], 0x24) == 0 &&
               word(fops[4], 0x30) == 0x25 &&
               word(fops[4], 0x28) == fake_record_addr &&
               word(fops[4], 0x2c) == 0);
    expect("fops_check_miss",
           check_file_plain(write_path, 0x25, 0x99) == false);
    allocator.forced_result = nullptr;
    wheel[5] = {};

    if (failures != 0) {
        std::printf("pool_wait_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("pool_wait_smoke: all checks passed\n");
    return 0;
}
