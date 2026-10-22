// 0x00715D0C contains exactly one function pointer, to 0x006B4A02;
// the following bytes belong to string data in the unprotected x86 PE.
// SHA-256: 41cbb7ad4b08f6edeb5c2a06aea037d1c0634e9b71df093cc9a26b2287916e10
#include "platform/HandleRegistry.h"

#include <cstdint>

extern void* PTR_FUN_00715d0c[1];

namespace {

struct SingleDestructorObject {
    void* vtable;

    void* destroy(std::uint32_t flags) {
        vtable = PTR_FUN_00715d0c;
        if ((flags & 1u) != 0) {
            hp6::platform::release_through_service(this);
        }
        return this;
    }
};

template <typename Member>
void* member_address(Member member) {
    union {
        Member method;
        void* address;
    } conversion{};
    conversion.method = member;
    return conversion.address;
}

} // namespace

void* PTR_FUN_00715d0c[1] = {member_address(&SingleDestructorObject::destroy)};
