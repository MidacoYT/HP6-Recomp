#include "platform/ServiceHolderDestructor.h"
#include "platform/ServiceHeapTeardown.h"

#include <cstdlib>

extern void* PTR_FUN_0083e55c[9];
extern void* PTR_LAB_008201e4[5];
extern void* PTR_LAB_008201f8[];
extern void* DAT_00cef6b0;
extern void* DAT_00cef6b4;
extern void* DAT_00cef6b8;
extern void* DAT_00fcdd74[6];

namespace hp6::platform {

void ServiceHolderObject::cleanup() {
    // Unprotected executable 0x005DB9A0..0x005DBA3F. The SEH prologue
    // and epilogue are supplied by the C++ compiler in this reconstruction.
    primary_vtable = PTR_FUN_0083e55c;
    secondary_vtable = PTR_FUN_0083e55c + 4;
    teardown_service_heap_005dd6b0(DAT_00cef6b0);
    if (DAT_00cef6b4 != nullptr) {
        teardown_service_heap_005dd6b0(DAT_00cef6b4);
    }
    if (DAT_00cef6b8 != nullptr) {
        teardown_service_heap_005dd6b0(DAT_00cef6b8);
    }
    std::free(allocation_at_0c);
    allocation_at_0c = nullptr;
    allocation_at_10 = nullptr;
    allocation_at_14 = nullptr;
    DAT_00cef6b0 = nullptr;
    DAT_00cef6b4 = nullptr;
    DAT_00cef6b8 = nullptr;
    secondary_vtable = PTR_LAB_008201e4;
    primary_vtable = PTR_LAB_008201f8;
}

} // namespace hp6::platform

void cleanup_service_holder_005db9a0(void* holder) {
    static_cast<hp6::platform::ServiceHolderObject*>(holder)->cleanup();
}

void cleanup_00711e60() {
    // 0x00711E60: mov ecx,0x00FCDD74; jmp 0x005DB9A0.
    cleanup_service_holder_005db9a0(DAT_00fcdd74);
}
