#pragma once
#include <cstdint>

namespace hp6::render {

// 0x01003650. Ghidra's exported signature is `void(void)`, but the
// exported x86 reads [EBP+8] and [EBP+0xc] in its only branch, so the
// binary's real (stdcall) signature takes two 32-bit arguments that are
// otherwise unused — evidence of under-counted parameters, as seen
// earlier for 0x01002890. Unlike the other functions in this batch, this
// is not a fatal-error call site: while DAT_00cf19a0 (likely a display
// mode enumeration counter) is at most 10, it formats
// "ModeOption:%d:%d by %d (%d bit)\n" with DAT_00866368, `width`,
// `height` and DAT_00cf19a8 — read left to right in that order from the
// format string, consistent with a per-mode enumeration log line
// ("ModeOption:<option>:<width> by <height> (<bit depth> bit)") capped at
// the first 10 modes. It is a no-op once the counter exceeds 10.
void report_mode_option(std::int32_t width, std::int32_t height);

} // namespace hp6::render
