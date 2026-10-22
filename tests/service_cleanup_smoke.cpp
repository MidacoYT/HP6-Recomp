// Local behavior checks for batch service_cleanup_038 (entry-table removal,
// three finalizers, three two-stage destroy variants). Game-space symbols
// get test-local definitions; the registry is exercised with stub tables.
// Not a binary comparison. x86 pointer layout required.
#include "platform/ServiceLifecycle.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

// Game-space equivalents (referenced by the library externs across the
// ServiceLifecycleExtra, ServiceLifecycle and HandleRegistry objects that
// the linked symbols pull in).
void* DAT_0089adcc = nullptr;
void* DAT_00fcdd74[6] = {};
void** DAT_00cef6bc = nullptr;
void* PTR_FUN_0083e55c[27] = {};
std::uint32_t DAT_00fd1254 = 0;
std::uint32_t DAT_008998c8 = 0;
void* PTR_FUN_007159b0 = nullptr;
extern void* PTR_FUN_00715d0c[1] = {};
extern void* PTR_LAB_00715da8[26] = {};
void* PTR_LAB_0071b500 = nullptr;
void* PTR_LAB_0071b540 = nullptr;
void* PTR_LAB_0071b520 = nullptr;
void* PTR_FUN_0071b628 = nullptr;
void* PTR_FUN_0071b62c = nullptr;
void* PTR_FUN_0071b650 = nullptr;
void* PTR_FUN_0071b670 = nullptr;
void* PTR_FUN_0071b5e8 = nullptr;
void* PTR_FUN_0071b608 = nullptr;
void* PTR_FUN_0071b560 = nullptr;
void* PTR_FUN_0071b580 = nullptr;
void* PTR_FUN_0071b5a0 = nullptr;

void cleanup_00711e60() {}

namespace {

int failures = 0;

void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

int member_slot20_calls = 0;
int member_slot8_calls = 0;
int mode_slot20_calls = 0;
int release_calls = 0;
void* released_block = nullptr;
std::uint32_t released_size = 0;

// Member slots: the binary pushes the object on the stack and lets the
// callee clean it up (006de5b0 assembly: PUSH EAX; CALL [ECX+0x20]) — the
// stubs must be __stdcall to keep the stack balanced.
#ifdef _MSC_VER
void __stdcall member_slot20(void*) { ++member_slot20_calls; }
void __stdcall member_slot8(void*) { ++member_slot8_calls; }
void __stdcall mode_slot20(void*) { ++mode_slot20_calls; }
#else
void member_slot20(void*) { ++member_slot20_calls; }
void member_slot8(void*) { ++member_slot8_calls; }
void mode_slot20(void*) { ++mode_slot20_calls; }
#endif

// Raw fake-vtable tables; slot 0 doubles as the vtable pointer at object + 0.
void* member_table[9]{};
void* mode_table[9]{};

// Allocator: a real class without virtual destructor keeps the release hook
// at vtable slot 1, matching the 006dcbc7 thiscall invocation.
struct Allocator {
    virtual void slot0() {}
    virtual void release(void* block, std::uint32_t size) {  // slot +0x04
        ++release_calls;
        released_block = block;
        released_size = size;
    }
};

void set_pointer(std::uint8_t* record, std::size_t offset, void* value) {
    *reinterpret_cast<void**>(record + offset) = value;
}

void* get_pointer(std::uint8_t* record, std::size_t offset) {
    return *reinterpret_cast<void**>(record + offset);
}

std::int32_t& word(std::uint8_t* record, std::size_t offset) {
    return *reinterpret_cast<std::int32_t*>(record + offset);
}

} // namespace

int main() {
    static_assert(sizeof(void*) == 4, "Service records store x86 pointers");
    using namespace hp6::platform;

    // ---- remove_entry_pointer (006dd00c) -------------------------------
    alignas(16) std::uint8_t owner[0xb0]{};
    std::int32_t tokens[4] = {0x11, 0x22, 0x33, 0x44};
    for (int i = 0; i < 4; ++i) word(owner, 0x20 + i * 4) = tokens[i];
    word(owner, 0xa0) = 4;
    word(owner, 0xa4) = 0x33;
    word(owner, 0xa8) = 0;
    word(owner, 0xac) = 0;

    expect("remove_found", remove_entry_pointer(owner, 0x22));
    expect("remove_count", word(owner, 0xa0) == 3);
    expect("remove_swap",
           word(owner, 0x20 + 1 * 4) == 0x44 && word(owner, 0x20 + 3 * 4) == 0x44);
    expect("remove_aux", word(owner, 0xa4) == 0x33);

    // Removing the value referenced by an auxiliary word clears only that
    // word; the clear order (0xac, 0xa8, 0xa4) is invisible when just one
    // word matches, so clear them one at a time across separate calls.
    word(owner, 0xa0) = 3;
    word(owner, 0xac) = tokens[3];
    expect("remove_clear_ac",
           remove_entry_pointer(owner, tokens[3]) && word(owner, 0xac) == 0);
    word(owner, 0xa0) = 2;
    word(owner, 0xa8) = 0x99;
    word(owner, 0x20) = 0x99;
    expect("remove_clear_a8", remove_entry_pointer(owner, 0x99) &&
                                 word(owner, 0xa8) == 0);
    expect("remove_missing", !remove_entry_pointer(owner, 0x777));
    expect("remove_null_value", !remove_entry_pointer(owner, 0));
    expect("remove_singleton_selfswap",
           [&] {
               alignas(16) std::uint8_t single[0xb0]{};
               word(single, 0x20) = 5;
               word(single, 0xa0) = 1;
               const bool ok = remove_entry_pointer(single, 5);
               return ok && word(single, 0xa0) == 0 &&
                      word(single, 0x20) == 5;
           }());

    // ---- finalize_record_0071b560 (006dd188) ---------------------------
    alignas(16) std::uint8_t record[0x4a8]{};
    // Member object at +0x468: a raw table whose slot 0 points at itself
    // (object + 0 holds the vtable pointer); slots +0x08 (index 2) and
    // +0x20 (index 8) hit the __stdcall stubs.
    member_table[0] = &member_table[0];
    member_table[2] = reinterpret_cast<void*>(&member_slot8);
    member_table[8] = reinterpret_cast<void*>(&member_slot20);
    set_pointer(record, 0x468, member_table);

    // Entry owner referenced from +0x464 holds the record itself.
    auto* entry_owner = static_cast<std::uint8_t*>(owner);
    word(owner, 0x20) = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(record));
    word(owner, 0xa0) = 1;
    set_pointer(record, 0x464, entry_owner);

    Allocator allocator;
    DAT_0089adcc = &allocator;

    finalize_record_0071b560(record);
    expect("final560_vtable",
           get_pointer(record, 0) == static_cast<void*>(&PTR_FUN_0071b650));
    expect("final560_member", member_slot20_calls == 1 && member_slot8_calls == 1);
    expect("final568_cleared", get_pointer(record, 0x468) == nullptr);
    expect("final560_entry_removed", word(owner, 0xa0) == 0);
    // destroy_record_with_member re-runs release_member_0x468 on the now
    // null member and, finding no member at +0x468, skips the service path.
    expect("final560_no_service", release_calls == 0);

    // ---- destroy_entry_record_0071b560 (006dd2c9) ----------------------
    std::memset(record, 0, sizeof record);
    set_pointer(record, 0x468, member_table);
    set_pointer(record, 0x464, entry_owner);
    word(owner, 0x20) = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(record));
    word(owner, 0xa0) = 1;
    const auto* returned = destroy_entry_record_0071b560(record, 1);
    expect("destroy560_return", returned == record);
    expect("destroy560_release",
           release_calls == 1 && released_block == record &&
               released_size == 0x46c);
    expect("destroy560_vtable",
           get_pointer(record, 0) == static_cast<void*>(&PTR_FUN_0071b650));

    // Same refit with flag 0: no allocator release.
    std::memset(record, 0, sizeof record);
    set_pointer(record, 0x468, member_table);
    set_pointer(record, 0x464, entry_owner);
    word(owner, 0x20) = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(record));
    word(owner, 0xa0) = 1;
    destroy_entry_record_0071b580(record, 0);
    expect("destroy580_norelease", release_calls == 1);
    expect("destroy580_vtable",
           get_pointer(record, 0) == static_cast<void*>(&PTR_FUN_0071b650));

    // ---- finalize_record_0071b5a0 (006ddb15): +0x498 slot 8 ------------
    std::memset(record, 0, sizeof record);
    member_slot20_calls = 0;
    member_slot8_calls = 0;
    mode_slot20_calls = 0;
    mode_table[0] = &mode_table[0];
    mode_table[8] = reinterpret_cast<void*>(&mode_slot20);
    set_pointer(record, 0x498, mode_table);
    set_pointer(record, 0x468, member_table);
    set_pointer(record, 0x464, entry_owner);
    word(owner, 0x20) = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(record));
    word(owner, 0xa0) = 1;
    finalize_record_0071b5a0(record);
    expect("final5a0_slot", mode_slot20_calls == 1 &&
                                get_pointer(record, 0x498) != nullptr);
    expect("final5a0_member_cleared", get_pointer(record, 0x468) == nullptr);
    expect("final5a0_entry_removed", word(owner, 0xa0) == 0);

    // ---- destroy_entry_record_0071b5a0 (006dddf6): release 0x4a4 -------
    std::memset(record, 0, sizeof record);
    set_pointer(record, 0x468, member_table);
    set_pointer(record, 0x464, entry_owner);
    word(owner, 0x20) = static_cast<std::int32_t>(
        reinterpret_cast<std::uintptr_t>(record));
    word(owner, 0xa0) = 1;
    destroy_entry_record_0071b5a0(record, 1);
    expect("destroy5a0_release",
           release_calls == 2 && released_size == 0x4a4);

    if (failures != 0) {
        std::printf("service_cleanup_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("service_cleanup_smoke: all checks passed\n");
    return 0;
}
