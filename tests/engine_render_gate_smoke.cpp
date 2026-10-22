#include "presentation/EngineRenderGate.h"

#include <cstdint>

std::uint8_t DAT_00866e28 = 1;
std::uint32_t DAT_00cef71c = 0;
int overlay_calls = 0;
void render_overlay_005dd020() { ++overlay_calls; }

int main() {
    hp6::presentation::EngineRenderGateObject gate{};
    gate.reset(1);
    if (DAT_00866e28 != 0 || DAT_00cef71c != 5) return 1;

    DAT_00866e28 = 1;
    gate.reset(0);
    if (DAT_00866e28 != 0 || DAT_00cef71c != 5) return 2;

    gate.reset(0xff);
    if (DAT_00cef71c != 5) return 3;
    for (int remaining = 4; remaining >= 1; --remaining) {
        gate.render(nullptr);
        if (DAT_00cef71c != static_cast<std::uint32_t>(remaining) ||
            DAT_00866e28 != 0 || overlay_calls != 0) return 4;
    }
    gate.render(nullptr);
    if (DAT_00cef71c != 0 || DAT_00866e28 != 1 || overlay_calls != 0) return 5;
    gate.render(nullptr);
    if (overlay_calls != 1) return 6;
    gate.reset(0);
    gate.render(nullptr);
    return DAT_00cef71c == 0 && overlay_calls == 1 ? 0 : 7;
}
