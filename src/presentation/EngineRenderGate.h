#pragma once

#include <cstdint>

namespace hp6::presentation {

// First word matches the object at 0x0087EADC. Its three vtable entry
// methods are reconstructed, but their deeper callees and object data remain.
struct EngineRenderGateObject {
    void* vtable;

    // Vtable slot 0, 0x005DCD80.
    void simulate(const void* tick);

    // Vtable slot +8, 0x005DCDA0. Uses the low byte of its stack argument.
    void reset(std::uint8_t restart);

    // Vtable slot +4, 0x005DCDC0. The tick pointer is not read by this method.
    void render(const void* tick);
};

} // namespace hp6::presentation
