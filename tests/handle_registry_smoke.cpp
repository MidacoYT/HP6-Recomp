// Local behavior checks for the handle registry batch (030). Game-space
// globals get test-local definitions arranged contiguously so the +0xc
// vtable-slot arithmetic behaves like the x86 image; on pointer sizes other
// than four bytes the harness reports a skip. Not a binary comparison.
#include "platform/HandleRegistry.h"
#include "platform/ServiceLifecycle.h"

#include <cstdio>
#include <cstring>
#include <cstdint>

#ifdef _WIN32
#include <windows.h>
#endif

namespace hp6::platform {
struct ExternBlock; // only for forward clarity; externs live in the TU below
}

// Contiguous game-space equivalents (x86-sized). Order matters: the service
// slot is read at &PTR_FUN_0083e55c + 0x0c.
void* PTR_FUN_0083e55c = nullptr;  // object pointer target
void* pad_0083e560_0 = nullptr;    // table +0x04
void* pad_0083e564_0 = nullptr;    // table +0x08
void* slot_0083e568_0 = nullptr;   // table +0x0c (service slot 3)
void* PTR_FUN_0083e56c = nullptr;
void* DAT_00fcdd74 = nullptr;
void* DAT_00fcdd78 = nullptr;
void** DAT_00cef6bc = nullptr;
std::uint32_t DAT_00fd1254 = 0;
void* PTR_FUN_007159b0 = nullptr;
void* PTR_FUN_00715d0c = nullptr;
void* PTR_LAB_00715da8 = nullptr;
void* PTR_LAB_0071b520 = nullptr;
void* PTR_LAB_0071b500 = nullptr;
void* PTR_LAB_0071b540 = nullptr;
void* DAT_0089adcc = nullptr;
void* PTR_FUN_0071b628 = nullptr;
void* PTR_FUN_0071b62c = nullptr;
void* PTR_FUN_0071b650 = nullptr;
void* PTR_FUN_0071b670 = nullptr;
void* PTR_FUN_0071b5e8 = nullptr;
void* PTR_FUN_0071b608 = nullptr;
// ServiceLifecycleExtra obj is linked for its destroy paths; its finalize
// externs need test-local definitions as well.
void* PTR_FUN_0071b560 = nullptr;
void* PTR_FUN_0071b580 = nullptr;
void* PTR_FUN_0071b5a0 = nullptr;

// The fixed-address in-block path uses this table's first word.
std::uint32_t DAT_008998c8 = 0;

namespace {

int cleanup_calls = 0;
int release_calls = 0;
void* release_object = nullptr;
void* release_holder = nullptr;
std::uint32_t release_mode = 0xFFFF;
void* out_block_marker = nullptr;
int layer_order[4]{};
int layer_calls = 0;
std::uint8_t* active_layer = nullptr;
int allocator_calls = 0;
void* allocator_this = nullptr;
void* allocator_block = nullptr;
std::uint32_t allocator_size = 0;
std::uint8_t* member_record = nullptr;
bool clear_member_on_first = false;
int member_first_calls = 0;
int member_second_calls = 0;

void __stdcall member_first(void*) {
    ++member_first_calls;
    if (clear_member_on_first) *reinterpret_cast<void**>(member_record + 0x468) = nullptr;
}
void __stdcall member_second(void*) { ++member_second_calls; }

struct LayerAllocator {
    virtual void slot0() {}
    virtual void release(void* block, std::uint32_t size) {
        ++allocator_calls; allocator_this = this;
        allocator_block = block; allocator_size = size;
    }
};

void __stdcall destroy_child(int flag) {
    if (flag == 3) layer_order[layer_calls++] = 1;
}
void __stdcall destroy_core_first(int flag) {
    if (flag == 1) layer_order[layer_calls++] = 3;
}
void __stdcall destroy_core_second(int flag) {
    if (flag == 1) {
        layer_order[layer_calls++] = 4;
        *reinterpret_cast<std::int32_t*>(active_layer + 0xa0) = 0;
    }
}
void __stdcall destroy_explicit(void* object) {
    if (object != nullptr) layer_order[layer_calls++] = 2;
}

struct TestService {
    virtual void slot0() {}
    virtual void slot1() {}
    virtual void slot2() {}
    virtual void release(void* object, std::uint32_t mode) {
        ++release_calls;
        release_holder = this;
        release_object = object;
        release_mode = mode;
    }
};

int failures = 0;

void expect(const char* label, bool condition) {
    if (!condition) {
        ++failures;
        std::printf("FAIL %s\n", label);
    }
}

} // namespace

void cleanup_00711e60() { ++cleanup_calls; }

int main() {
    if (sizeof(void*) != 4) {
        std::printf("handle_registry_smoke: skipped (x86 semantics only)\n");
        return 0;
    }

    // Contiguous slot installs the release stub at +0x0c. Global emission
    // order is implementation-defined, so verify adjacency first.
    TestService service;
    auto* vtable = *reinterpret_cast<void***>(&service);
    slot_0083e568_0 = vtable[3];
    const auto* slot_read =
        reinterpret_cast<void* const*>(&PTR_FUN_0083e55c)[3];
    if (slot_read != vtable[3]) {
        std::printf("handle_registry_smoke: skipped (global layout adjacency)\n");
        return 0;
    }

    // One-shot holder initialization.
    expect("holder_first",
           hp6::platform::service_holder() ==
               reinterpret_cast<void**>(&DAT_00fcdd74));
    expect("holder_flags", (DAT_00fd1254 & 1u) != 0u);
    expect("holder_object",
           DAT_00fcdd74 == static_cast<void*>(&PTR_FUN_0083e55c));
    expect("holder_second",
           DAT_00fcdd78 == static_cast<void*>(&PTR_FUN_0083e56c));
    expect("holder_watcher",
           DAT_00cef6bc == reinterpret_cast<void**>(&DAT_00fcdd74));
    // cleanup_00711e60 is registered through the CRT thunk; atexit fires
    // after main, so only the registration side effect is observable here.
    expect("holder_stable",
           hp6::platform::service_holder() ==
               reinterpret_cast<void**>(&DAT_00fcdd74));

    // Virtual release through slot +0xc with (object, 0).
    std::uint32_t object = 0x12345678u;
    hp6::platform::release_through_service(&object);
    expect("release_object", release_object == &object);
    expect("release_holder", release_holder == &DAT_00fcdd74);
    expect("release_mode", release_mode == 0u);
    expect("release_calls", release_calls == 1);

    // Close outside the static block goes through the service slot.
    std::uint32_t outside = 0u;
    hp6::platform::close_handle(&outside);
    expect("close_outside_release", release_calls == 2);
    expect("close_outside_object", release_object == &outside);
    expect("close_outside_mode", release_mode == 0u);

    hp6::platform::close_handle(nullptr);
    expect("close_null_no_release", release_calls == 2);

    // Exercise the game's fixed in-block address when that region is free.
#ifdef _WIN32
    void* mapped = ::VirtualAlloc(reinterpret_cast<void*>(0x00890000u),
                                  0x10000, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
    if (mapped == reinterpret_cast<void*>(0x00890000u)) {
        auto* in_block = reinterpret_cast<std::uint32_t*>(0x00896498u);
        *in_block = 0;
        DAT_008998c8 = 0x12345678u;
        hp6::platform::close_handle(in_block);
        expect("in_block_atomic_zero", DAT_008998c8 == 0);
        expect("in_block_no_release", release_calls == 2);
        std::printf("handle_registry_smoke: fixed-address branch checked\n");
        ::VirtualFree(mapped, 0, MEM_RELEASE);
    } else {
        std::printf("handle_registry_smoke: fixed-address branch unavailable\n");
    }
#endif

    const auto far_out = reinterpret_cast<std::uint32_t*>(&out_block_marker);
    hp6::platform::close_handle(far_out);
    expect("close_far_release", release_calls == 3);
    expect("close_far_object", release_object == far_out);

    // Four constructor variants install distinct vtables.
    void* store[4] = {};
    expect("ctor1",
           hp6::platform::construct_vtable_007159b0(store, 1) == store);
    expect("ctor1_vt", store[0] == &PTR_FUN_007159b0);
    expect("ctor1_release", release_calls == 4);
    hp6::platform::construct_vtable_00715d0c(store + 1, 0);
    expect("ctor2_vt", store[1] == &PTR_FUN_00715d0c);
    expect("ctor2_flag0_norelease", release_calls == 4);
    hp6::platform::construct_vtable_00715da8(store + 2, 1);
    expect("ctor3_vt", store[2] == &PTR_LAB_00715da8);
    expect("ctor3_release", release_calls == 5);
    hp6::platform::construct_vtable_0071b520(store + 3, 1);
    expect("ctor4_vt", store[3] == &PTR_LAB_0071b520);
    expect("ctor4_release", release_calls == 6);

    LayerAllocator allocator;
    DAT_0089adcc = &allocator;
    std::uintptr_t child_table[1] = {reinterpret_cast<std::uintptr_t>(destroy_child)};
    std::uintptr_t core_first_table[1] = {reinterpret_cast<std::uintptr_t>(destroy_core_first)};
    std::uintptr_t core_second_table[1] = {reinterpret_cast<std::uintptr_t>(destroy_core_second)};
    std::uintptr_t explicit_table[3] = {0, 0,
        reinterpret_cast<std::uintptr_t>(destroy_explicit)};
    std::uint32_t child_words[2] = {1, static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(child_table))};
    std::uintptr_t core_first = reinterpret_cast<std::uintptr_t>(core_first_table);
    std::uintptr_t core_second = reinterpret_cast<std::uintptr_t>(core_second_table);
    std::uintptr_t explicit_object = reinterpret_cast<std::uintptr_t>(explicit_table);
    alignas(16) std::uint8_t layer[0xc0]{};
    active_layer = layer;
    *reinterpret_cast<void**>(layer + 0xb8) = &child_words[1];
    *reinterpret_cast<void**>(layer + 0xb4) = &explicit_object;
    *reinterpret_cast<void**>(layer + 0xb0) = &core_first;
    *reinterpret_cast<void**>(layer + 0x20) = &core_second;
    *reinterpret_cast<std::int32_t*>(layer + 0xa0) = 1;
    expect("destroy_layer_return", hp6::platform::destroy_layer_0xc0(layer, 1) == layer);
    expect("layer_call_order", layer_calls == 4 && layer_order[0] == 1 &&
                               layer_order[1] == 2 && layer_order[2] == 3 &&
                               layer_order[3] == 4);
    expect("layer_cleared", *reinterpret_cast<void**>(layer + 0xb4) == nullptr &&
                            *reinterpret_cast<std::int32_t*>(layer + 0xa0) == 0 &&
                            *reinterpret_cast<void**>(layer) == &PTR_LAB_0071b520);
    expect("layer_allocator", allocator_calls == 1 && allocator_this == &allocator &&
                              allocator_block == layer && allocator_size == 0xc0);

    layer_calls = 0;
    *reinterpret_cast<void**>(layer + 0xb0) = &core_first;
    *reinterpret_cast<void**>(layer + 0x20) = &core_second;
    *reinterpret_cast<std::int32_t*>(layer + 0xa0) = 1;
    expect("destroy_layer_b4_return", hp6::platform::destroy_layer_0xb4(layer, 1) == layer);
    expect("destroy_layer_b4_calls", layer_calls == 2 && layer_order[0] == 3 &&
                                  layer_order[1] == 4 && allocator_calls == 2 &&
                                  allocator_size == 0xb4);

    alignas(16) std::uint8_t small_record[0x464]{};
    expect("destroy_24", hp6::platform::destroy_record_0x24(small_record, 1) == small_record &&
                         *reinterpret_cast<void**>(small_record) == &PTR_FUN_0071b628 &&
                         allocator_size == 0x24 && allocator_calls == 3);
    expect("destroy_78", hp6::platform::destroy_record_0x78(small_record, 1) == small_record &&
                         *reinterpret_cast<void**>(small_record) == &PTR_FUN_0071b670 &&
                         allocator_size == 0x78 && allocator_calls == 4);
    expect("destroy_464", hp6::platform::destroy_record_0x464(small_record, 1) == small_record &&
                          *reinterpret_cast<void**>(small_record) == &PTR_FUN_0071b650 &&
                          allocator_size == 0x464 && allocator_calls == 5);
    expect("destroy_04", hp6::platform::destroy_record_0x04(small_record, 1) == small_record &&
                         *reinterpret_cast<void**>(small_record) == &PTR_FUN_0071b670 &&
                         allocator_size == 4 && allocator_calls == 6);
    hp6::platform::reset_record_vtable_0071b670(small_record);
    expect("reset_vtable", *reinterpret_cast<void**>(small_record) == &PTR_FUN_0071b670);

    expect("destroy_464_two_stage",
           hp6::platform::destroy_record_0x464_two_stage(small_record, 1) == small_record &&
           *reinterpret_cast<void**>(small_record) == &PTR_FUN_0071b650 &&
           allocator_size == 0x464 && allocator_calls == 7);

    alignas(16) std::uint8_t large_record[0x46c]{};
    member_record = large_record;
    std::uintptr_t member_table[9]{};
    member_table[2] = reinterpret_cast<std::uintptr_t>(member_second);
    member_table[8] = reinterpret_cast<std::uintptr_t>(member_first);
    std::uintptr_t member_object = reinterpret_cast<std::uintptr_t>(member_table);
    *reinterpret_cast<void**>(large_record + 0x468) = &member_object;
    clear_member_on_first = true;
    expect("member_clear_return", hp6::platform::release_member_0x468(large_record) == 0);
    expect("member_clear_short", member_first_calls == 1 && member_second_calls == 0 &&
                                 *reinterpret_cast<void**>(large_record + 0x468) == nullptr);

    *reinterpret_cast<void**>(large_record + 0x468) = &member_object;
    clear_member_on_first = false;
    expect("destroy_46c_return", hp6::platform::destroy_record_0x46c(large_record, 1) == large_record);
    expect("member_two_calls", member_first_calls == 2 && member_second_calls == 1 &&
                               *reinterpret_cast<void**>(large_record + 0x468) == nullptr);
    expect("destroy_46c_final", *reinterpret_cast<void**>(large_record) == &PTR_FUN_0071b650 &&
                                 allocator_calls == 8 && allocator_size == 0x46c);

    if (failures != 0) {
        std::printf("handle_registry_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("handle_registry_smoke: all checks passed\n");
    return 0;
}

