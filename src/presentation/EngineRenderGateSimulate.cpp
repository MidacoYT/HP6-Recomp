#include "presentation/EngineRenderGate.h"
#include "presentation/EngineEventSnapshot.h"

extern void* DAT_00fcaf34;
// 0x005DCB30: sends pending tick messages; its deeper callees remain open.
extern void dispatch_engine_tick_005dcb30();

namespace hp6::presentation {

void EngineRenderGateObject::simulate(const void* tick) {
    (void)tick;
    // 0x005DCD80..0x005DCD92: load DAT_00FCAF34 into ESI, call 0x005E03C0,
    // then 0x005DCB30. Neither method reads the explicit tick pointer.
    snapshot_engine_events(DAT_00fcaf34);
    dispatch_engine_tick_005dcb30();
}

} // namespace hp6::presentation
