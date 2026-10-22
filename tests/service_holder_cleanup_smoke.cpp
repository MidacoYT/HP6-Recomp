#include "platform/ServiceHolderDestructor.h"

#include <cstdlib>

void* PTR_FUN_0083e55c[27] = {};
void* PTR_LAB_008201e4[5] = {};
void* PTR_LAB_008201f8[1] = {};
void* DAT_00cef6b0 = nullptr;
void* DAT_00cef6b4 = nullptr;
void* DAT_00cef6b8 = nullptr;
void* DAT_00fcdd74[6] = {};

namespace {
void* visited[3] = {};
int calls = 0;
bool vtables_installed = true;
}

std::uint32_t teardown_service_heap_005dd6b0(void* heap) {
    auto* holder = reinterpret_cast<hp6::platform::ServiceHolderObject*>(DAT_00fcdd74);
    vtables_installed &= holder->primary_vtable == PTR_FUN_0083e55c &&
                         holder->secondary_vtable == PTR_FUN_0083e55c + 4;
    if (calls < 3) visited[calls] = heap;
    ++calls;
    return 1;
}

void cleanup_00711e60();

int main() {
    auto* holder = reinterpret_cast<hp6::platform::ServiceHolderObject*>(DAT_00fcdd74);
    holder->allocation_at_0c = std::malloc(16);
    if (holder->allocation_at_0c == nullptr) return 1;
    holder->allocation_at_10 = reinterpret_cast<void*>(0x11);
    holder->allocation_at_14 = reinterpret_cast<void*>(0x22);
    DAT_00cef6b0 = reinterpret_cast<void*>(0x100);
    DAT_00cef6b4 = reinterpret_cast<void*>(0x200);
    DAT_00cef6b8 = reinterpret_cast<void*>(0x300);

    cleanup_00711e60();
    if (calls != 3 || visited[0] != reinterpret_cast<void*>(0x100) ||
        visited[1] != reinterpret_cast<void*>(0x200) ||
        visited[2] != reinterpret_cast<void*>(0x300) || !vtables_installed) return 2;
    if (holder->primary_vtable != PTR_LAB_008201f8 ||
        holder->secondary_vtable != PTR_LAB_008201e4 ||
        holder->allocation_at_0c != nullptr || holder->allocation_at_10 != nullptr ||
        holder->allocation_at_14 != nullptr || DAT_00cef6b0 != nullptr ||
        DAT_00cef6b4 != nullptr || DAT_00cef6b8 != nullptr) return 3;

    cleanup_00711e60();
    return calls == 4 && visited[0] == reinterpret_cast<void*>(0x100) ? 0 : 4;
}
