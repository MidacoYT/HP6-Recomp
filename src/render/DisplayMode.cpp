#include "render/DisplayMode.h"
#include "platform/Diagnostics.h"

// Addresses read by the binary but absent from the exported globals; the
// final binary must supply their stored values explicitly.
// DAT_00cf19a0 is the same global hp6::runtime::copy_fixed_records reads
// (likely a shared mode/record count); its role here is this enumeration
// cap only.
extern std::int32_t DAT_00cf19a0;
extern std::int32_t DAT_00866368; // option index; exact meaning beyond this print is unresolved
extern std::int32_t DAT_00cf19a8; // bit depth

namespace hp6::render {

// ReAgent candidate: reports/pipeline/candidates/01003650.cpp (adjusted
// for the two real parameters established above).
void report_mode_option(std::int32_t width, std::int32_t height) {
    if (DAT_00cf19a0 > 10) {
        return;
    }
    hp6::platform::format_diagnostic_message("ModeOption:%d:%d by %d (%d bit)\n",
                                              DAT_00866368, width, height,
                                              DAT_00cf19a8);
}

} // namespace hp6::render
