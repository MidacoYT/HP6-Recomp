// Verified in both HP6 x86 PEs: the word at 0x0087EADC is 0x0083E5C8.
// The 12 bytes at 0x0083E5C8 are 0x005DCD80, 0x005DCDC0,
// 0x005DCDA0; the following bytes are string data.
// This reconstructs the first interface pointer only. The adjacent object
// fields at 0x0087EAE0 and beyond are separate relocation work.

#include "presentation/EngineRenderGate.h"

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

void* engine_render_gate_vtable_0083e5c8[3] = {
    member_address(&hp6::presentation::EngineRenderGateObject::simulate),
    member_address(&hp6::presentation::EngineRenderGateObject::render),
    member_address(&hp6::presentation::EngineRenderGateObject::reset),
};

} // namespace

void* PTR_PTR_0087eadc = engine_render_gate_vtable_0083e5c8;
