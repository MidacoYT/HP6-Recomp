// The two contiguous service interfaces at 0x0083E55C and 0x0083E56C
// have exactly 4 + 5 dwords in both verified HP6 x86 executable images.
// Original slot addresses: 005DB970, 005DB7A0, 005DB710, 005DB8D0,
// 005DB800, 005E00C0, 00407B30, 00407B30, 005E0080.
#include "platform/ServiceHolderDestructor.h"
#include "runtime/VerifiedBinaryLeafs.h"

namespace {
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

void* PTR_FUN_0083e55c[9] = {
    member_address(&hp6::platform::ServiceHolderObject::destroy),
    member_address(&hp6::platform::ServiceHolderObject::allocate_named),
    member_address(&hp6::platform::ServiceHolderObject::allocate_context),
    member_address(&hp6::platform::ServiceHolderObject::free_block),
    member_address(&hp6::platform::ServiceHolderSecondary::allocate_from_options),
    member_address(&hp6::platform::ServiceHolderSecondary::free_block),
    reinterpret_cast<void*>(&hp6::binary_leafs::fn_00407b30),
    reinterpret_cast<void*>(&hp6::binary_leafs::fn_00407b30),
    member_address(&hp6::platform::ServiceHolderSecondary::destroy),
};
