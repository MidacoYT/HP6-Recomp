#include "platform/ServiceLifecycle.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

void* DAT_00892df8 = nullptr;
void* PTR_LAB_00715d30 = nullptr;
void* PTR_FUN_00715d0c = nullptr;
void* PTR_LAB_00715e18 = nullptr;
void* PTR_LAB_00715da8 = nullptr;

namespace {
int failures = 0;
int order[4]{};
int calls = 0;
int releases = 0;
void* release_this = nullptr;
void* released_block = nullptr;
std::uint32_t released_size = 0;

void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

struct Component {
    explicit Component(int value) : id(value) {}
    int id;
    virtual void slot0() {}
    virtual void slot1() {}
    virtual void slot2() { order[calls++] = id; }
    virtual void slot3() { order[calls++] = id; }
};

struct Allocator {
    virtual void slot0() {}
    virtual void release(void* block, std::uint32_t size) {
        ++releases;
        release_this = this;
        released_block = block;
        released_size = size;
    }
};

void set_pointer(std::uint8_t* record, std::size_t offset, void* value) {
    *reinterpret_cast<void**>(record + offset) = value;
}
}

int main() {
    static_assert(sizeof(void*) == 4, "Service records store x86 pointers");
    using namespace hp6::platform;
    Allocator allocator;
    DAT_00892df8 = &allocator;
    Component a{1}, b{2}, c{3}, d{4};
    alignas(16) std::uint8_t record[0x84]{};
    set_pointer(record, 0x14, &a);
    set_pointer(record, 8, &b);
    set_pointer(record, 0x0c, &c);
    set_pointer(record, 4, &d);

    expect("destroy_84_return", destroy_record_0x84(record, 1) == record);
    expect("component_order", calls == 4 && order[0] == 1 && order[1] == 2 &&
                              order[2] == 3 && order[3] == 4);
    expect("variant_vtable", *reinterpret_cast<void**>(record) == &PTR_LAB_00715da8 &&
                             *reinterpret_cast<void**>(record + 0x1c) == &PTR_FUN_00715d0c);
    expect("allocator_this", release_this == &allocator);
    expect("allocator_args", releases == 1 && released_block == record &&
                             released_size == 0x84);

    calls = 0;
    expect("destroy_1c_return", destroy_record_0x1c(record, 0) == record);
    expect("destroy_1c_calls", calls == 4 && releases == 1);
    expect("destroy_1c_vtable", *reinterpret_cast<void**>(record) == &PTR_LAB_00715da8);

    std::printf("service_lifecycle_smoke: %d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
