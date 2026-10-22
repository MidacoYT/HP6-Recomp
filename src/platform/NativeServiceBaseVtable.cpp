// 0x008201E4..0x008201F7 in both HP6 x86 PE images: four _purecall
// entries followed by 0x005E0830. The next table begins at 0x008201F8.
// Unprotected PE SHA-256:
// 41cbb7ad4b08f6edeb5c2a06aea037d1c0634e9b71df093cc9a26b2287916e10

#include "platform/HandleRegistry.h"

#include <cstdint>

extern "C" int __cdecl _purecall();
extern void* PTR_LAB_008201e4[5];

namespace {

struct ServiceBaseObject {
    void* vtable;

    void* destroy(std::uint32_t flags) {
        // 0x005E0830..0x005E0854: install this base vtable, then use the
        // holder's virtual release slot when the low flag bit is set.
        vtable = PTR_LAB_008201e4;
        if ((flags & 1u) != 0u) {
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

void* PTR_LAB_008201e4[5] = {
    reinterpret_cast<void*>(&_purecall),
    reinterpret_cast<void*>(&_purecall),
    reinterpret_cast<void*>(&_purecall),
    reinterpret_cast<void*>(&_purecall),
    member_address(&ServiceBaseObject::destroy),
};
