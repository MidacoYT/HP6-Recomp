#include "presentation/EngineRenderGate.h"

// 0x005DD020 remains unresolved: it formats and renders the native overlay.
// It must be reconstructed before this method can enter the native vtable.
extern void render_overlay_005dd020();
extern std::uint8_t DAT_00866e28;
extern std::uint32_t DAT_00cef71c;

namespace hp6::presentation {

void EngineRenderGateObject::render(const void* tick) {
    (void)tick;
    // Unprotected executable 0x005DCDC0..0x005DCDEB. The nonzero path
    // returns immediately, including on the transition from 1 to 0.
    if (DAT_00cef71c != 0) {
        --DAT_00cef71c;
        DAT_00866e28 = (DAT_00cef71c == 0) ? 1 : 0;
        return;
    }
    if (DAT_00866e28 != 0) {
        render_overlay_005dd020();
    }
}

} // namespace hp6::presentation
