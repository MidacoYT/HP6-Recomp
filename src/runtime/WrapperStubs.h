#pragma once

#include <cstdint>

namespace hp6::runtime {

// No-op methods of the DirectDraw-style wrapper family. Byte evidence:
// 006b54af RET 8, 006b54b2 RET 4, 006b54b5 RET 4, 006b54bb RET 4 and
// 006b54be RET write nothing; several are the tail-jump targets of the
// this-adjust forwarders listed below (PUSH EBP; MOV EBP,ESP;
// MOV ECX,[ECX]; POP EBP; JMP) that re-derive the wrapped object from the
// wrapper before forwarding. The stack-word count of each cleaned frame
// (stdcall-style RET n) is mirrored by the trailing word arguments.

// 006b54bb (stdcall, one stack word): reached directly and through the
// 006b44ee adjust-forwarder.
void wrap_noop_006b54bb(std::uint8_t* object, std::uint32_t arg);

// 006b54be (cdecl, reached from 006b6306 with `this` left in ECX).
void wrap_noop_006b54be(std::uint8_t* object);

// 006b454a (forwarder to 006b54b2: adjust, then no-op, one stack word).
void wrap_noop_006b454a(std::uint8_t* wrapper, std::uint32_t arg);

// 006b4555 (forwarder to 006b54b5: adjust, then no-op, one stack word).
void wrap_noop_006b4555(std::uint8_t* wrapper, std::uint32_t arg);

// 006b4560 (forwarder to 006b54af: adjust, then no-op, two stack words).
void wrap_noop_006b4560(std::uint8_t* wrapper, std::uint32_t arg_1,
                        std::uint32_t arg_2);

// 006b54b8 (cdecl, MOV AL,1 / RET): constant-true stub consulted by the
// window message handlers of the same wrapper family.
bool wrap_true_006b54b8();

// 006b45ae (fastcall in the binary, the block in ECX): rewrites the
// 0x16-byte state header of the wrapper family. Byte evidence:
// OR [ECX],-1 / OR [ECX+4],-1, then dwords +8/+0xc/+0x10 and the bytes
// at +0x14/+0x15 are zeroed.
void reset_wrapper_block_006b45ae(std::uint8_t* block);

} // namespace hp6::runtime
