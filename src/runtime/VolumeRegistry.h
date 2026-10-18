#pragma once

#include <cstdint>

namespace hp6::runtime {

// 006b3b0e (cdecl, caller cleans). Clears `bytes` bytes at `buffer` to
// zero. The exported x86 tail-calls the CRT memset slot PTR_FUN_008516a8
// whose target is the static CRT memset body compiled at 006b3c8c; the
// reconstructed call sites use std::memset directly.
void clear_buffer(std::uintptr_t buffer, std::uint32_t bytes);

// 006b25b8 (cdecl). Resolves the volume record serving `path`.
// - No ':' in the path and no leading '/' or '\\': returns the word at
//   root + 0x98 as-is (the engine slot base).
// - Without a colon but with a path separator: the drive prefix is taken
//   from the game path `*(*ptr to game path)` reached twice through
//   root + 0x88.
// - With a colon: the prefix comes from `path` itself.
// In both prefix cases a 16-byte local is zeroed first (006b3b0e) and then
// strncpy fills len(colon)+1 bytes, so the standard NUL padding of
// strncpy is impossible and the zeroed tail provides the terminator.
// The volume list head lives at root + 0x7c and records chain through
// their first dword. For each record, the object at +0x84 answers its
// name through the vtable slot +0x30 — thiscall, `this` in ECX, the name
// buffer as one stack argument cleaned by the callee (exported x86: PUSH
// buf; MOV EAX,[ECX]; CALL [EAX+0x30]) — and the result is compared with
// _stricmp. Returns the matching record pointer or null.
std::uint8_t* find_volume_by_name(const char* path);

// 006b2673 (true tail call: PUSH EBP; MOV EBP,ESP; PUSH [EBP+8]; CALL
// find_volume_by_name; POP ECX; MOV ECX,[EAX+0x84]; MOV EAX,[ECX]; POP EBP;
// JMP [EAX+0x2c] — EBP is restored before the jump, so the callee's own
// epilogue returns straight to this function's caller). Resolves the
// volume record for `path`, then calls vtable slot +0x2c of its object
// (record+0x84), thiscall with the object as `this` and no stack
// arguments. The exported decompiler could not recover the indirect
// jump's target and read it as a 0-argument call; there is no null check
// on the resolved record, matching the exported x86 exactly.
void invoke_volume_method_2c(const char* path);

} // namespace hp6::runtime
