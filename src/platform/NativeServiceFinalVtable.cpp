// 0x008201F8..0x00820207 in both verified HP6 x86 PE images:
// 0x005A2450, then three 0x006B92B0 (_purecall) slots. The next
// dword at 0x00820208 starts a different vtable (0x004089B0).
// Unprotected PE SHA-256:
// 41cbb7ad4b08f6edeb5c2a06aea037d1c0634e9b71df093cc9a26b2287916e10

#include "platform/HandleRegistry.h"

#include <cstdint>

extern "C" int __cdecl _purecall();
extern void* PTR_LAB_008201f8[4];

namespace {
struct ServiceFinalObject {
    void* vtable;

    void* destroy(std::uint32_t flags) {
        // 0x005A2450..0x005A2474: restore this vtable and release the
        // object through 0x005DB920's holder slot +0xC when bit 0 is set.
        vtable = PTR_LAB_008201f8;
        if ((flags & 1u) != 0)
            hp6::platform::release_through_service(this);
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

void* PTR_LAB_008201f8[4] = {
    member_address(&ServiceFinalObject::destroy),
    reinterpret_cast<void*>(&_purecall),
    reinterpret_cast<void*>(&_purecall),
    reinterpret_cast<void*>(&_purecall),
};
