#include "platform/ServiceLifecycle.h"
#include "platform/HandleRegistry.h"

#include <cstdint>
#include <cstddef>

extern void* DAT_0089adcc;
extern void* PTR_LAB_0071b500;
extern void* PTR_LAB_0071b540;
extern void* PTR_LAB_0071b520;
extern void* PTR_FUN_0071b628;
extern void* PTR_FUN_0071b62c;
extern void* PTR_FUN_0071b650;
extern void* PTR_FUN_0071b670;
extern void* PTR_FUN_0071b5e8;
extern void* PTR_FUN_0071b608;
extern void* PTR_FUN_0071b560;
extern void* PTR_FUN_0071b580;
extern void* PTR_FUN_0071b5a0;

namespace hp6::platform {
namespace {

#ifdef _MSC_VER
using ReleaseCall = void (__thiscall*)(void*, void*, std::uint32_t);
using DestroyCall = void (__thiscall*)(void*, std::uint32_t);
using ExplicitObjectCall = void (__stdcall*)(void*);
#else
using ReleaseCall = void (*)(void*, void*, std::uint32_t);
using DestroyCall = void (*)(void*, std::uint32_t);
using ExplicitObjectCall = void (*)(void*);
#endif

void destroy_with_flag(void* object, std::uint32_t flag) {
    auto* table = *reinterpret_cast<std::uintptr_t**>(object);
    reinterpret_cast<DestroyCall>(table[0])(object, flag);
}

} // namespace

void release_secondary_block(void* block, std::uint32_t size) {
    auto* allocator = DAT_0089adcc;
    auto* table = *reinterpret_cast<std::uintptr_t**>(allocator);
    reinterpret_cast<ReleaseCall>(table[1])(allocator, block, size);
}

void destroy_layer_core(std::uint8_t* record) {
    auto* current = *reinterpret_cast<void**>(record + 0xb0);
    *reinterpret_cast<void**>(record) = &PTR_LAB_0071b540;
    while (true) {
        if (current != nullptr) destroy_with_flag(current, 1);
        if (*reinterpret_cast<std::int32_t*>(record + 0xa0) < 1) break;
        current = *reinterpret_cast<void**>(record + 0x20);
    }
    *reinterpret_cast<std::uint32_t*>(record + 0xa0) = 0;
    *reinterpret_cast<void**>(record) = &PTR_LAB_0071b520;
}

void destroy_layer_with_children(std::uint8_t* record) {
    auto* first = *reinterpret_cast<std::uint8_t**>(record + 0xb8);
    *reinterpret_cast<void**>(record) = &PTR_LAB_0071b500;
    if (first != nullptr) {
        auto* preceding = reinterpret_cast<std::uint32_t*>(first - 4);
        if (*preceding == 0) release_through_service(preceding);
        else destroy_with_flag(first, 3);
    }
    auto* second = *reinterpret_cast<void**>(record + 0xb4);
    if (second != nullptr) {
        auto* table = *reinterpret_cast<std::uintptr_t**>(second);
        reinterpret_cast<ExplicitObjectCall>(table[2])(second);
        *reinterpret_cast<void**>(record + 0xb4) = nullptr;
    }
    destroy_layer_core(record);
}

std::uint8_t* destroy_layer_0xc0(std::uint8_t* record, std::uint8_t flag) {
    destroy_layer_with_children(record);
    if ((flag & 1u) != 0) release_secondary_block(record, 0xc0);
    return record;
}

std::uint8_t* destroy_layer_0xb4(std::uint8_t* record, std::uint8_t flag) {
    destroy_layer_core(record);
    if ((flag & 1u) != 0) release_secondary_block(record, 0xb4);
    return record;
}

std::uint8_t* destroy_record_0x24(std::uint8_t* record, std::uint8_t flag) {
    *reinterpret_cast<void**>(record) = &PTR_FUN_0071b628;
    if ((flag & 1u) != 0) release_secondary_block(record, 0x24);
    return record;
}

void reset_record_vtable_0071b670(std::uint8_t* record) {
    *reinterpret_cast<void**>(record) = &PTR_FUN_0071b670;
}

std::uint8_t* destroy_record_0x78(std::uint8_t* record, std::uint8_t flag) {
    *reinterpret_cast<void**>(record) = &PTR_FUN_0071b62c;
    reset_record_vtable_0071b670(record);
    if ((flag & 1u) != 0) release_secondary_block(record, 0x78);
    return record;
}

std::uint8_t* destroy_record_0x464(std::uint8_t* record, std::uint8_t flag) {
    *reinterpret_cast<void**>(record) = &PTR_FUN_0071b650;
    if ((flag & 1u) != 0) release_secondary_block(record, 0x464);
    return record;
}

std::uint8_t* destroy_record_0x04(std::uint8_t* record, std::uint8_t flag) {
    *reinterpret_cast<void**>(record) = &PTR_FUN_0071b670;
    if ((flag & 1u) != 0) release_secondary_block(record, 4);
    return record;
}

void set_vtable_0071b650(std::uint8_t* record) {
    *reinterpret_cast<void**>(record) = &PTR_FUN_0071b650;
}

std::uint8_t* destroy_record_0x464_two_stage(std::uint8_t* record,
                                               std::uint8_t flag) {
    *reinterpret_cast<void**>(record) = &PTR_FUN_0071b5e8;
    set_vtable_0071b650(record);
    if ((flag & 1u) != 0) release_secondary_block(record, 0x464);
    return record;
}

std::uint32_t release_member_0x468(std::uint8_t* record) {
    auto* object = *reinterpret_cast<void**>(record + 0x468);
    if (object != nullptr) {
        auto* table = *reinterpret_cast<std::uintptr_t**>(object);
        reinterpret_cast<ExplicitObjectCall>(table[0x20 / 4])(object);
        object = *reinterpret_cast<void**>(record + 0x468);
        if (object != nullptr) {
            table = *reinterpret_cast<std::uintptr_t**>(object);
            reinterpret_cast<ExplicitObjectCall>(table[8 / 4])(object);
            *reinterpret_cast<void**>(record + 0x468) = nullptr;
        }
    }
    return 0;
}

void destroy_record_with_member(std::uint8_t* record) {
    *reinterpret_cast<void**>(record) = &PTR_FUN_0071b608;
    release_member_0x468(record);
    auto* member = *reinterpret_cast<void**>(record + 0x468);
    if (member != nullptr) {
        release_through_service(member);
        *reinterpret_cast<void**>(record + 0x468) = nullptr;
    }
    *reinterpret_cast<void**>(record) = &PTR_FUN_0071b5e8;
    set_vtable_0071b650(record);
}

std::uint8_t* destroy_record_0x46c(std::uint8_t* record, std::uint8_t flag) {
    destroy_record_with_member(record);
    if ((flag & 1u) != 0) release_secondary_block(record, 0x46c);
    return record;
}

// 006dd00c. Swap-removal from the entry table at + 0x20 (count at + 0xa0).
// The exported x86 confirms a bool return in AL: XOR AL,AL on the early-out
// and not-found paths, MOV AL,1 on removal. The scan re-reads the count each
// iteration (CMP EDI,[ECX+0xa0] in the loop), reproduced here. The Ghidra
// CONCAT31 / "value & 0xffffff00" forms of the decompile are artifacts of
// untracked EAX upper bits. Field clears run in the fixed order 0xac, 0xa8,
// 0xa4 and the removed slot receives the last entry of the pre-removal
// array.
bool remove_entry_pointer(void* owner, std::int32_t value) {
    if (value == 0) {
        return false;
    }
    auto* record = static_cast<std::uint8_t*>(owner);
    auto* entry = reinterpret_cast<std::int32_t*>(record + 0x20);
    for (std::int32_t index = 0;
         index < *reinterpret_cast<std::int32_t*>(record + 0xa0);
         ++index, ++entry) {
        if (*entry == value) {
            const std::int32_t last_index =
                *reinterpret_cast<std::int32_t*>(record + 0xa0) - 1;
            *reinterpret_cast<std::int32_t*>(record + 0xa0) = last_index;
            *entry = *reinterpret_cast<std::int32_t*>(record + last_index * 4 +
                                                      0x20);
            // Fixed clear order: 0xac, then 0xa8, then 0xa4.
            const std::size_t auxiliaries[3] = {0xac, 0xa8, 0xa4};
            for (std::size_t slot = 0; slot < 3; ++slot) {
                auto* word = reinterpret_cast<std::int32_t*>(
                    record + auxiliaries[slot]);
                if (*word == value) {
                    *word = 0;
                }
            }
            return true;
        }
    }
    return false;
}

// 006dd188. First-stage finalize: install the 0071b560 vtable, release the
// +0x468 member through its virtual table, drop the record from the member
// object's entry table at +0x464 (no null check exists in the binary), and
// run destroy_record_with_member.
void finalize_record_0071b560(std::uint8_t* record) {
    *reinterpret_cast<void**>(record) = &PTR_FUN_0071b560;
    release_member_0x468(record);
    remove_entry_pointer(
        *reinterpret_cast<void**>(record + 0x464),
        static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(record)));
    destroy_record_with_member(record);
}

// 006dd354. Twin finalize with the 0071b580 vtable.
void finalize_record_0071b580(std::uint8_t* record) {
    *reinterpret_cast<void**>(record) = &PTR_FUN_0071b580;
    release_member_0x468(record);
    remove_entry_pointer(
        *reinterpret_cast<void**>(record + 0x464),
        static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(record)));
    destroy_record_with_member(record);
}

// 006ddb15. Finalize with the 0071b5a0 vtable: the +0x498 member receives a
// vtable slot +0x20 (index 8) call before the shared cleanup steps.
void finalize_record_0071b5a0(std::uint8_t* record) {
    *reinterpret_cast<void**>(record) = &PTR_FUN_0071b5a0;
    auto* target = *reinterpret_cast<void**>(record + 0x498);
    if (target != nullptr) {
        auto* table = *reinterpret_cast<std::uintptr_t**>(target);
        reinterpret_cast<ExplicitObjectCall>(table[0x20 / 4])(target);
    }
    release_member_0x468(record);
    remove_entry_pointer(
        *reinterpret_cast<void**>(record + 0x464),
        static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(record)));
    destroy_record_with_member(record);
}

// 006dd2c9 / 006dd894 / 006dddf6. Two-stage destroy variants; the record
// size released under flag bit 0 comes from each call site.
std::uint8_t* destroy_entry_record_0071b560(std::uint8_t* record,
                                             std::uint8_t flag) {
    finalize_record_0071b560(record);
    if ((flag & 1u) != 0) release_secondary_block(record, 0x46c);
    return record;
}

std::uint8_t* destroy_entry_record_0071b580(std::uint8_t* record,
                                             std::uint8_t flag) {
    finalize_record_0071b580(record);
    if ((flag & 1u) != 0) release_secondary_block(record, 0x46c);
    return record;
}

std::uint8_t* destroy_entry_record_0071b5a0(std::uint8_t* record,
                                             std::uint8_t flag) {
    finalize_record_0071b5a0(record);
    if ((flag & 1u) != 0) release_secondary_block(record, 0x4a4);
    return record;
}

} // namespace hp6::platform
