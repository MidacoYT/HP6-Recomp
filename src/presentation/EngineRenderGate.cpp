#include "presentation/EngineRenderGate.h"

// Both initial values are read from the original PE and the unprotected PE.
// 0x00866E28 starts at 1; 0x00CEF71C starts at 0.
extern std::uint8_t DAT_00866e28;
extern std::uint32_t DAT_00cef71c;

namespace hp6::presentation {

void EngineRenderGateObject::reset(std::uint8_t restart) {
    // Unprotected executable 0x005DCDA0..0x005DCDB9:
    // xor al,al; cmp [esp+4],al; mov [00866e28],al;
    // je return; mov dword [00cef71c],5; ret 4.
    DAT_00866e28 = 0;
    if (restart != 0) {
        DAT_00cef71c = 5;
    }
}

} // namespace hp6::presentation
