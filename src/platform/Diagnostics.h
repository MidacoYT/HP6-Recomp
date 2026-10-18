#pragma once

namespace hp6::platform {

// 0x0063ccf0 (cdecl varargs). Formats `format` and its arguments with
// vsprintf into a 2048-byte local buffer and returns. Ghidra marks this
// noreturn, but the exported x86 is exactly MOV/SUB/LEA/PUSH*3/CALL
// vsprintf/ADD/RET — the formatted buffer is never used for anything else
// (no write, no abort/exit call) and the function does return. Every
// caller observed so far reaches this only from a branch that has nothing
// to fall through to afterward, so the discrepancy with Ghidra's noreturn
// tag has no observable effect on any integrated caller.
void format_diagnostic_message(const char* format, ...);

} // namespace hp6::platform
