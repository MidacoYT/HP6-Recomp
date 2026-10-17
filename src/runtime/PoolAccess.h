#pragma once

// Service-pool read paths from the HP6 binary exports. ReAgent was
// unavailable for this batch (LLM usage limit); integrations follow the
// exported decompilations. Equivalence remains unverified.

#include <cstdint>

namespace hp6::runtime {

// 006b18df. Slot address arithmetic over the global service root
// (DAT_00892b60): base = *(root + 0x98); result = base + (index & 0xff)*0x88;
// when the byte at result + 0x80 is nonzero and `reset` is nonzero, the
// plain base is returned instead. The original propagates the value as an
// int; the address form here keeps that arithmetic bit-exact.
std::uintptr_t pool_slot_address(std::uint32_t index, bool reset);

// 006b1910. Increment the root counter under its lock and combine its
// previous value with the signed slot index derived from base +0x98.
std::uint32_t allocate_slot_token(std::uintptr_t slot_address);

// 006b203a. Engine-thread bootstrap over the service-pool root
// (DAT_00892b60). When the record is already flagged at +0x80, the flag at
// +4 is set to 1 and the tail call jumps to vtable slot +4 of the object
// at +0x84 (__thiscall, ECX = that object; exported x86 shows POP EDI/ESI
// then JMP [EAX+4]). Otherwise the engine builds its locks/events, installs
// the 0xff wake word at +0x7c, launches the worker through the shared
// launcher (callback 006b1e32, CREATE_SUSPENDED, priority code 1), waits on
// the manual-reset event at +0x78, resets it, and leaves the root lock.
void bootstrap_engine(std::uintptr_t record);

// 006b20e8. Engine attach constructor (thiscall): vtable 007159e8; boots
// the engine at param_3 when its +4 flag is zero, then fills the fields
// (+0x10..+0x30, table pointer DAT_006b18de at +0x1c) and reads the engine
// token via 006b1910 into +0xc. Returns the receiver.
std::uint8_t* attach_engine(std::uint8_t* receiver, std::uint8_t mode,
                            std::uint32_t words, std::uintptr_t engine);

// 006b2553 (cdecl). Runs a callback under the timer wheel (or record) lock
// with the reserve word at +0x7c visibly held during the call; the
// callback is cdecl with (reserve value, argument) and returns its own
// value, which this function returns after the reserve is restored and
// the auto-reset event at +0x48 is signalled.
std::uint32_t run_reserved_callback(
    std::uint32_t (*callback)(std::uint32_t, std::int32_t),
    std::uintptr_t record, std::uint32_t reserve_value,
    std::int32_t callback_arg);
std::uint32_t run_reserved_callback(
    void (*callback)(std::uint32_t, std::int32_t), std::uintptr_t record,
    std::uint32_t reserve_value, std::int32_t callback_arg);

// 006d9351. CRT thread trampoline (start address of the launcher): sets the
// first context word to 1 (the wake flag the launcher waits for), then
// invokes the callback stored at +4 with the argument at +8. stdcall in the
// binary (callee cleans the single pointer argument).
void thread_trampoline_006d9351(void* context);

// 006b1a2e. Virtual read at frame: uses the object pointer at +0x10 to
// reach vtable slot 0x24, calls it with the value stored at +8, forwards
// the 64-bit return to `out_value`, and copies the frame words at +0x18 and
// +0x1c into `out_words[0]` and `out_words[1]`. The original calling
// convention of the slot passes the object in ECX and the word at +8 on
// the stack; the C++ call mirrors this on MSVC x86.
void read_virtual_pair(const std::uint8_t* frame, std::uint64_t* out_value,
                       std::uint32_t* out_words);

} // namespace hp6::runtime
