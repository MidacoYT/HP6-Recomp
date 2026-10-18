#include "runtime/PoolList.h"

#include "runtime/PoolAccess.h"
#include "platform/Sync.h"

#include <cstddef>
#include <cstdint>

extern std::int32_t DAT_00892b60;
// Engine attach constructor externs reused by the node constructors below.
extern void* PTR_FUN_00715ac0;
extern void* PTR_FUN_00715ae0;
extern void* PTR_FUN_00715ad0;
extern void* PTR_FUN_00715b00;
extern void* PTR_FUN_00715af0;

namespace hp6::runtime {
namespace {

std::uint32_t& word(std::uint8_t* bytes, std::size_t offset) {
    return *reinterpret_cast<std::uint32_t*>(bytes + offset);
}

std::uint8_t* pointer(std::uint32_t address) {
    return reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(address));
}

std::uint32_t address(const std::uint8_t* bytes) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(bytes));
}

std::uint8_t* next_node(std::uint8_t* node) {
    const auto link = word(node, 8);
    return link == 0 ? nullptr : pointer(link - 8);
}

} // namespace

std::uint8_t* detach_first(std::uint8_t* list) {
    auto* first = pointer(word(list, 0));
    if (first != nullptr) {
        if (address(first) == word(list, 4)) {
            word(list, 4) = 0;
            word(list, 0) = 0;
        } else {
            word(list, 0) = address(next_node(first));
        }
        --word(list, 8);
        word(first, 8) = 0;
    }
    return first; // EAX retains the original first node in 006b2cbd.
}

void insert_first(std::uint8_t* list, std::uint8_t* node) {
    const auto old_first = word(list, 0);
    word(node, 8) = old_first == 0 ? 0 : old_first + 8;
    ++word(list, 8);
    word(list, 0) = address(node);
    if (word(node, 8) == 0) word(list, 4) = address(node);
}

void insert_after(std::uint8_t* list, std::uint8_t* previous,
                  std::uint8_t* node) {
    if (previous == nullptr) {
        insert_first(list, node);
        return;
    }
    if (word(previous, 8) == 0) word(list, 4) = address(node);
    word(node, 8) = word(previous, 8);
    word(previous, 8) = address(node) + 8;
    ++word(list, 8);
}

std::uint8_t* detach_first_locked(std::uint32_t /*allocation_size*/) {
    // The size word is pushed by every observed caller and ignored by the
    // callee (006b1d53 assembly reads no [ESP+x] and returns with a bare
    // RET, leaving the cleanup to the cdecl caller).
    auto* root = pointer(static_cast<std::uint32_t>(DAT_00892b60));
    platform::enter_lock(reinterpret_cast<std::uintptr_t>(root + 0x0c));
    // The assembly reloads DAT_00892b60 after locking.
    auto* list = pointer(static_cast<std::uint32_t>(DAT_00892b60));
    auto* first = detach_first(list);
    platform::leave_lock(reinterpret_cast<std::uintptr_t>(list + 0x0c));
    return first;
}

bool detach_node(std::uint8_t* list, std::uint8_t* node,
                 std::uint8_t* previous) {
    auto* first = pointer(word(list, 0));
    if (node == first) {
        --word(list, 8);
        if (address(node) == word(list, 4)) {
            word(list, 0) = 0;
            word(list, 4) = 0;
        } else {
            word(list, 0) = address(next_node(node));
        }
    } else {
        if (first == nullptr) return false;
        if (previous == nullptr) previous = first;
        const auto target_link = node == nullptr ? 0u : address(node) + 8u;
        while (word(previous, 8) != 0 && word(previous, 8) != target_link) {
            previous = next_node(previous);
        }
        if (word(previous, 8) == 0 || word(previous, 8) != target_link) {
            return false;
        }
        --word(list, 8);
        word(previous, 8) = word(node, 8);
        if (address(node) == word(list, 4)) {
            word(list, 4) = address(previous);
        }
    }
    word(node, 8) = 0;
    return true;
}

std::uint8_t* find_node(std::uint8_t* list, std::int32_t key, bool detach) {
    std::uint8_t* previous = nullptr;
    for (auto* node = pointer(word(list, 0)); node != nullptr;
         previous = node, node = next_node(node)) {
        if (static_cast<std::int32_t>(word(node, 0x0c)) == key) {
            if (detach) detach_node(list, node, previous);
            return node;
        }
    }
    return nullptr;
}

std::uint8_t* find_pool_node(std::uint8_t* frame, std::int32_t key,
                             std::uint32_t* source_code) {
    *source_code = 0;
    if (word(frame, 4) == 0) return nullptr;
    auto* current = pointer(word(frame, 0x0c));
    if (current != nullptr && static_cast<std::int32_t>(word(current, 0x0c)) == key) {
        *source_code = 0xffffffffu;
        return current;
    }
    auto* found = find_node(frame + 0x10, key, false);
    if (found != nullptr) {
        *source_code = 1;
        return found;
    }
    return find_node(frame + 0x1c, key, false);
}

std::int32_t pool_node_status(std::uint32_t index) {
    auto* frame = reinterpret_cast<std::uint8_t*>(pool_slot_address(index, true));
    platform::enter_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
    std::uint32_t source_code = 0;
    auto* node = find_pool_node(frame, static_cast<std::int32_t>(index), &source_code);
    std::int32_t result = 0;
    if (source_code == 0) {
        if (node == nullptr) result = -3;
        else if (static_cast<std::int8_t>(node[0x10]) == 1) result = -1;
        else result = static_cast<std::int8_t>(node[0x11]);
    }
    platform::leave_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
    return result;
}

void schedule_node(std::uint8_t* node) {
    auto* frame = reinterpret_cast<std::uint8_t*>(
        pool_slot_address(word(node, 0x0c), true));
    platform::enter_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
    const auto sort_key = [](std::uint8_t* item) {
        return (static_cast<std::uint32_t>(item[0x12]) << 24) |
               (word(item, 0x0c) & 0x00ffff00u);
    };
    const auto key = sort_key(node);
    std::uint8_t* previous = nullptr;
    for (auto* current = pointer(word(frame, 0x10)); current != nullptr &&
         sort_key(current) <= key; current = next_node(current)) {
        previous = current;
    }
    insert_after(frame + 0x10, previous, node);
    platform::leave_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
    platform::signal_event(reinterpret_cast<std::uintptr_t>(frame + 0x44));
}

void update_node_priority(std::uint32_t index, std::uint8_t priority) {
    auto* frame = reinterpret_cast<std::uint8_t*>(pool_slot_address(index, true));
    if (frame == nullptr) return;
    platform::enter_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
    auto* node = find_node(frame + 0x10, static_cast<std::int32_t>(index), true);
    platform::leave_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
    if (node != nullptr) {
        node[0x12] = priority;
        schedule_node(node);
    }
}

void notify_pool_node(std::uint32_t index) {
    auto* frame = reinterpret_cast<std::uint8_t*>(pool_slot_address(index, true));
    platform::enter_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
    std::uint32_t source_code = 0;
    auto* node = find_pool_node(frame, static_cast<std::int32_t>(index), &source_code);
    if (node != nullptr && source_code != 0) {
#ifdef _MSC_VER
        using Method = void (__thiscall*)(void*);
#else
        using Method = void (*)(void*);
#endif
        auto* vtable = *reinterpret_cast<std::uintptr_t**>(node);
        reinterpret_cast<Method>(vtable[3])(node);
    }
    platform::leave_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
}

std::uint64_t take_completed_node(std::uint32_t index) {
    auto* frame = reinterpret_cast<std::uint8_t*>(pool_slot_address(index, true));
    platform::enter_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
    auto* node = find_node(frame + 0x1c, static_cast<std::int32_t>(index), true);
    platform::leave_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
#ifdef _MSC_VER
    using ReadMethod = std::uint64_t (__thiscall*)(void*);
    using DeleteMethod = void (__thiscall*)(void*, int);
#else
    using ReadMethod = std::uint64_t (*)(void*);
    using DeleteMethod = void (*)(void*, int);
#endif
    auto* vtable = *reinterpret_cast<std::uintptr_t**>(node);
    const auto result = reinterpret_cast<ReadMethod>(vtable[2])(node);
    reinterpret_cast<DeleteMethod>(vtable[0])(node, 1);
    return result;
}

void set_pool_callback(std::uint32_t index, NodeCallback callback) {
    auto* frame = reinterpret_cast<std::uint8_t*>(pool_slot_address(index, true));
    platform::enter_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
    std::uint32_t source_code = 0;
    auto* node = find_pool_node(frame, static_cast<std::int32_t>(index), &source_code);
    if (source_code == 0) {
        callback(word(node, 0x0c), static_cast<std::int8_t>(node[0x11]),
                 word(node, 0x18));
    } else {
        word(node, 0x1c) = static_cast<std::uint32_t>(
            reinterpret_cast<std::uintptr_t>(callback));
    }
    platform::leave_lock(reinterpret_cast<std::uintptr_t>(frame + 0x28));
}

// 006b3032 (thiscall in the binary; receiver as first argument here).
// Attaches the pooled engine (006b20e8), installs the vtable 00715ac0,
// stores the record word at +0x14 and schedules the node (006b2199).
std::uint8_t* construct_timer_node(std::uint8_t* receiver, std::uint32_t id,
                                   std::uint8_t mode, std::uint32_t words,
                                   std::uintptr_t engine) {
    attach_engine(receiver, mode, words, engine);
    *reinterpret_cast<void**>(receiver) = &PTR_FUN_00715ac0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x14) = id;
    schedule_node(receiver);
    return receiver;
}

// 006b318b (thiscall in the binary). Same attach order; the attach words
// land at +0x18 through 006b20e8, +0x38/+0x3c are cleared, the scheduling
// words fill +0x14/+0x28/+0x2c/+0x20/+0x24 in the decompiled order, the
// vtable 00715ae0 is installed, +0x30 is written last and the node is
// scheduled (006b2199).
std::uint8_t* construct_repeat_node(std::uint8_t* receiver, std::uint32_t word_0x14,
                                    std::uint32_t field_0x28,
                                    std::uint32_t field_0x2c,
                                    std::uint32_t field_0x20,
                                    std::uint32_t field_0x24,
                                    std::uint32_t field_0x30,
                                    std::uint8_t mode,
                                    std::uint32_t attach_words,
                                    std::uintptr_t engine) {
    attach_engine(receiver, mode, attach_words, engine);
    *reinterpret_cast<std::uint32_t*>(receiver + 0x38) = 0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x3c) = 0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x14) = word_0x14;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x28) = field_0x28;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x2c) = field_0x2c;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x20) = field_0x20;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x24) = field_0x24;
    *reinterpret_cast<void**>(receiver) = &PTR_FUN_00715ae0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x30) = field_0x30;
    schedule_node(receiver);
    return receiver;
}

// 006b2456 (cdecl). Pulls a 0x40-byte slot from the global pool (the size
// word is unread by 006b1d53), builds the timed node from the record and
// returns its +0xc word. The binary has no empty-pool guard: the null
// slot path falls through to MOV EAX,[EAX+0xc] and faults, exactly like
// the original; no synthesized handling is added.
std::uint32_t create_node_from_record(std::uintptr_t record, std::uint8_t mode,
                                      std::uint32_t words) {
    std::uint8_t* slot = detach_first_locked(0x40);
    const auto* source = reinterpret_cast<const std::uint8_t*>(record);
    construct_timer_node(slot, static_cast<std::uint32_t>(record), mode, words,
                         *reinterpret_cast<const std::uint32_t*>(source + 0xc));
    return *reinterpret_cast<std::uint32_t*>(slot + 0xc);
}

// 006b3094 (thiscall in the binary). Engine attach, +0x38/+0x3c cleared,
// record word at +0x14, bounded words at +0x28/+0x2c and +0x20/+0x24 with
// sign words (param >> 31), vtable 00715ad0, +0x30, then scheduling.
std::uint8_t* construct_signed_node(std::uint8_t* receiver, std::uint32_t id,
                                    std::int32_t bound_low,
                                    std::int32_t bound_high,
                                    std::uint32_t field_0x30,
                                    std::uint8_t mode,
                                    std::uint32_t attach_words,
                                    std::uintptr_t engine) {
    attach_engine(receiver, mode, attach_words, engine);
    *reinterpret_cast<std::uint32_t*>(receiver + 0x38) = 0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x3c) = 0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x14) = id;
    *reinterpret_cast<std::int32_t*>(receiver + 0x28) = bound_low;
    *reinterpret_cast<std::int32_t*>(receiver + 0x2c) = bound_low >> 31;
    *reinterpret_cast<std::int32_t*>(receiver + 0x20) = bound_high;
    *reinterpret_cast<void**>(receiver) = &PTR_FUN_00715ad0;
    *reinterpret_cast<std::int32_t*>(receiver + 0x24) = bound_high >> 31;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x30) = field_0x30;
    schedule_node(receiver);
    return receiver;
}

// 006b335c (thiscall in the binary). Engine attach first, then the vtable
// 00715b00 is written BEFORE the record word at +0x14 (different order
// from the other constructors), and the node is scheduled.
std::uint8_t* construct_plain_node(std::uint8_t* receiver, std::uint32_t id,
                                   std::uint8_t mode, std::uint32_t attach_words,
                                   std::uintptr_t engine) {
    attach_engine(receiver, mode, attach_words, engine);
    *reinterpret_cast<void**>(receiver) = &PTR_FUN_00715b00;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x14) = id;
    schedule_node(receiver);
    return receiver;
}

// 006b24b7 (cdecl). Same shape over 006b318b with the decompiled argument
// funnel: (param_1, param_5, param_6, param_2, param_3, param_4, param_7,
// param_8, record + 0xc). Like 006b2456, no empty-pool guard: the null
// slot path reads [null+0xc] exactly as the binary does.
std::uint32_t create_repeat_node_from_record(
    std::uintptr_t record, std::uint32_t field_0x28, std::uint32_t field_0x2c,
    std::uint32_t field_0x20, std::uint32_t field_0x24, std::uint32_t field_0x30,
    std::uint8_t mode, std::uint32_t attach_words) {
    std::uint8_t* slot = detach_first_locked(0x40);
    const auto* source = reinterpret_cast<const std::uint8_t*>(record);
    construct_repeat_node(slot, static_cast<std::uint32_t>(record), field_0x28,
                          field_0x2c, field_0x20, field_0x24, field_0x30, mode,
                          attach_words,
                          *reinterpret_cast<const std::uint32_t*>(source + 0xc));
    return *reinterpret_cast<std::uint32_t*>(slot + 0xc);
}

// 006b2482 (cdecl). Same 0x40 slot pull; builds a signed node over the
// decompiled funnel (param_1, param_4, param_2, param_3, param_5, param_6,
// record + 0xc). As with the other creators, the null slot path reads
// [null+0xc] exactly like the binary.
std::uint32_t create_signed_node_from_record(
    std::uintptr_t record, std::uint32_t bound_high, std::uint32_t field_0x30,
    std::uint32_t bound_low, std::uint8_t mode, std::uint32_t attach_words) {
    std::uint8_t* slot = detach_first_locked(0x40);
    const auto* source = reinterpret_cast<const std::uint8_t*>(record);
    construct_signed_node(
        slot, static_cast<std::uint32_t>(record),
        static_cast<std::int32_t>(bound_low),
        static_cast<std::int32_t>(bound_high), field_0x30, mode, attach_words,
        *reinterpret_cast<const std::uint32_t*>(source + 0xc));
    return *reinterpret_cast<std::uint32_t*>(slot + 0xc);
}

// 006b2527 (cdecl). 0x40 slot pull + 006b335c plain constructor with the
// record + 0xc engine; null slot path identical to the binary.
std::uint32_t create_plain_node_from_record(std::uintptr_t record,
                                            std::uint8_t mode,
                                            std::uint32_t attach_words) {
    std::uint8_t* slot = detach_first_locked(0x40);
    const auto* source = reinterpret_cast<const std::uint8_t*>(record);
    construct_plain_node(
        slot, static_cast<std::uint32_t>(record), mode, attach_words,
        *reinterpret_cast<const std::uint32_t*>(source + 0xc));
    return *reinterpret_cast<std::uint32_t*>(slot + 0xc);
}

// 006b329a (thiscall in the binary, callee-clean RET 0x1c). The exported
// x86 pushes mode/words/engine, calls the engine attach (006b20e8), then
// fills +0x14, the two signed CDQ pairs (+0x28/+0x2c and +0x20/+0x24,
// interleaved with the vtable write), the unsigned +0x30, and schedules
// last. The binary stores the vtable address 0x715af0 directly.
std::uint8_t* construct_probe_node_00715af0(
    std::uint8_t* receiver, std::uint32_t record, std::int32_t value_0x28,
    std::int32_t value_0x20, std::uint32_t field_0x30, std::uint8_t mode,
    std::uint32_t attach_words, std::uintptr_t engine) {
    attach_engine(receiver, mode, attach_words, engine);
    *reinterpret_cast<std::uint32_t*>(receiver + 0x14) = record;
    // CDQ of each signed argument becomes the pair's high word.
    const std::uint32_t high_0x28 = value_0x28 < 0 ? 0xffffffffu : 0u;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x28) =
        static_cast<std::uint32_t>(value_0x28);
    *reinterpret_cast<std::uint32_t*>(receiver + 0x2c) = high_0x28;
    const std::uint32_t high_0x20 = value_0x20 < 0 ? 0xffffffffu : 0u;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x20) =
        static_cast<std::uint32_t>(value_0x20);
    *reinterpret_cast<void**>(receiver) = &PTR_FUN_00715af0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x24) = high_0x20;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x30) = field_0x30;
    schedule_node(receiver);
    return receiver;
}

// 006b24f2 (cdecl). 0x40 slot pull + 006b329a with the decompiled funnel
// 329a(slot, param_1, param_4, param_2, param_3, param_5, param_6,
// *(param_1 + 0xc)); null slot path reads [null+0xc] exactly like the
// binary. Returns the node's +0xc token.
std::uint32_t create_probe_node_from_record(
    std::uintptr_t record, std::int32_t value_0x20, std::uint32_t field_0x30,
    std::int32_t value_0x28, std::uint8_t mode, std::uint32_t attach_words) {
    std::uint8_t* slot = detach_first_locked(0x40);
    const auto* source = reinterpret_cast<const std::uint8_t*>(record);
    construct_probe_node_00715af0(
        slot, static_cast<std::uint32_t>(record), value_0x28, value_0x20,
        field_0x30, mode, attach_words,
        *reinterpret_cast<const std::uint32_t*>(source + 0xc));
    return *reinterpret_cast<std::uint32_t*>(slot + 0xc);
}

} // namespace hp6::runtime
