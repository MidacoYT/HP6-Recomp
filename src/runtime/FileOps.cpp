#include "runtime/FileOps.h"

#include "runtime/PoolAccess.h"
#include "runtime/PoolWait.h"
#include "runtime/VolumeRegistry.h"

// Game-space externs read by the exported x86 of the reserved workers:
// the error-report object (also reached by 006b1968), the source line
// word placed in the allocate descriptor, and the hlsfile.cpp source
// literal shared by the 006da2xx/006da7xx families.
extern void* DAT_00851384;
extern std::int32_t DAT_008513ac;
extern const char hlsfile_source_0071b358[];

namespace hp6::runtime {

namespace {

// The vtable slot 0 call of the error-report object: thiscall with two
// stack arguments (request word, descriptor pointer), cleaned by the
// callee - the same shape 006b1968 uses.
void* allocate_error_report(std::uint32_t request, void* descriptor) {
    auto* object =
        reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(DAT_00851384));
    const auto vtable = static_cast<std::uintptr_t>(
        *(reinterpret_cast<const std::uint32_t*>(object)));
#ifdef _MSC_VER
    using AllocateCall = void* (__thiscall*)(void* self, std::uint32_t length,
                                             void* request);
#else
    using AllocateCall = void* (*)(void* self, std::uint32_t length,
                                   void* request);
#endif
    return reinterpret_cast<AllocateCall>(*reinterpret_cast<void**>(vtable))(
        object, request, descriptor);
}

} // namespace

// 006d9f47 (cdecl). [EBP+0x8] = reserve, [ESI] = context. The exported x86:
//   EAX = 006b175b(*ESI, 1, reserve)          rec 1 (bounded path, pair low)
//   EAX = 006b1815(rec1, reserve-1)           rec 2 (plain, pair low)
//   descriptor [EBP-0x4c..-0x8] = {6, 0xac, &5, 5, hlsfile.cpp, &7, 7,
//       &ctx[4], &1, 1 (EDI), path, &flag_b, (ctx4>>9)&2, DAT_008513ac,
//       &flag_a, (ctx4>>6)&4, 1, 0}
//   EDI = (**vtable of DAT_00851384)(rec2, &descriptor)
//   EDI != 0: 006b1783(rec1, 0, EDI, rec2, reserve-1); 006b17e5(rec1,
//             reserve-1); ctx[2] = rec2; return EDI
//   else:     006b17e5(rec1, reserve-1); return 0
std::uint32_t open_file_reserved(std::uint32_t reserve,
                                 std::int32_t context_word) {
    auto* ctx = reinterpret_cast<std::uint32_t*>(context_word);
    const std::int64_t path_pair = create_and_wait_bounded_path_node(
        reinterpret_cast<const char*>(ctx[0]), 1,
        static_cast<std::uint8_t>(reserve));
    // The bounded-path wait answers the 64-bit pair; the low word is the
    // record the remaining stages chain on (the binary reads the low EAX).
    const std::uintptr_t record_1 = static_cast<std::uintptr_t>(path_pair);
    if (record_1 == 0) {
        return 0;
    }

    const std::int64_t plain_pair = create_and_wait_plain_node(
        record_1, static_cast<std::uint8_t>(reserve - 1));
    const std::uint32_t record_2 = static_cast<std::uint32_t>(plain_pair);
    // [EBP+0x8] kept the second record; the context words +0xc/+0x8 are
    // the descriptor tail the binary clears after the wait (AND -0x8,0).
    const std::uint32_t context_4 = ctx[4];

    std::uint32_t descriptor[18] = {        6,                                        // [EBP-0x4c]
        0xac,                                     // [EBP-0x48]
        reinterpret_cast<std::uint32_t>(&descriptor[3]),
        5,                                        // [EBP-0x40]
        reinterpret_cast<std::uint32_t>(hlsfile_source_0071b358),
        reinterpret_cast<std::uint32_t>(&descriptor[6]),
        7,                                        // [EBP-0x34]
        reinterpret_cast<std::uint32_t>(ctx + 4), // &context[4]
        reinterpret_cast<std::uint32_t>(&descriptor[9]),
        1,                                        // [EBP-0x28] (EDI)
        ctx[0],                                   // path
        reinterpret_cast<std::uint32_t>(&descriptor[12]),
        (context_4 >> 9) & 2,                     // [EBP-0x1c]
        static_cast<std::uint32_t>(DAT_008513ac), // [EBP-0x18]
        reinterpret_cast<std::uint32_t>(&descriptor[15]),
        (context_4 >> 6) & 4,                     // [EBP-0x10]
        1,                                        // [EBP-0xc] (EDI)
        0,                                        // [EBP-0x8]
    };
    const std::uint32_t answer = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(
            allocate_error_report(record_2, descriptor)));
    if (answer != 0) {
        create_and_wait_repeat_node(record_1, 0, answer,
                                    static_cast<std::int32_t>(record_2),
                                    static_cast<std::uint8_t>(reserve - 1));
        create_and_wait_node(record_1, static_cast<std::uint8_t>(reserve - 1));
        ctx[2] = record_2;
        return answer;
    }
    create_and_wait_node(record_1, static_cast<std::uint8_t>(reserve - 1));
    return 0;
}

// 006da0c0 (cdecl). [ESI] = context, EDI = reserve:
//   EBX = EDI = reserve; rec1 = 006b175b(*ESI, 1, reserve)
//   rec1 != 0: 006b1783(rec1, 0, ctx[1], ctx[2], reserve-1); ESI = pair low
//   006b17e5(rec1, reserve-1) (answer ignored); return the repeat pair low
std::uint32_t write_file_reserved(std::uint32_t reserve,
                                  std::int32_t context_word) {
    auto* ctx = reinterpret_cast<std::uint32_t*>(context_word);
    const std::int64_t path_pair = create_and_wait_bounded_path_node(
        reinterpret_cast<const char*>(ctx[0]), 1,
        static_cast<std::uint8_t>(reserve));
    const std::uintptr_t record_1 = static_cast<std::uintptr_t>(path_pair);
    if (record_1 == 0) {
        return 0;
    }
    const std::int64_t write_pair = create_and_wait_repeat_node(
        record_1, 0, ctx[1], static_cast<std::int32_t>(ctx[2]),
        static_cast<std::uint8_t>(reserve - 1));
    create_and_wait_node(record_1, static_cast<std::uint8_t>(reserve - 1));
    return static_cast<std::uint32_t>(write_pair);
}

// 006da462 (cdecl). [ESI] = context, EDI = reserve:
//   rec1 = 006b175b(*ESI, 6, reserve)
//   rec1 != 0: 006b17b7(rec1, 0, ctx[1], ctx[2], reserve-1); EBX = pair low
//   006b17e5(rec1, reserve-1) (answer ignored); SETZ over EBX == ctx[2]
std::uint32_t check_file_reserved(std::uint32_t reserve,
                                  std::int32_t context_word) {
    auto* ctx = reinterpret_cast<std::uint32_t*>(context_word);
    const std::int64_t path_pair = create_and_wait_bounded_path_node(
        reinterpret_cast<const char*>(ctx[0]), 6,
        static_cast<std::uint8_t>(reserve));
    const std::uintptr_t record_1 = static_cast<std::uintptr_t>(path_pair);
    if (record_1 == 0) {
        return 0;
    }
    const std::int64_t probe_pair = create_and_wait_probe_node(
        record_1, 0, ctx[1], static_cast<std::int32_t>(ctx[2]),
        static_cast<std::uint8_t>(reserve - 1));
    create_and_wait_node(record_1, static_cast<std::uint8_t>(reserve - 1));
    return static_cast<std::uint32_t>(probe_pair) == ctx[2] ? 1u : 0u;
}

namespace {

// Shared reserved-call entry: the wrappers push {context, 0x64, record}
// under the callback address, exactly like the exported x86
// (LEA ECX,[EBP-0x14]; PUSH ECX; PUSH 0x64; CALL 006b25b8; POP ECX;
//  PUSH EAX; CALL 006b2553). The worker and callback types agree at the
// machine level, so the casts mirror the binary's word passing.
template <typename Worker>
std::uint32_t run_file_op(const Worker worker, const char* path,
                          std::uint32_t* context) {
    return run_reserved_callback(
        reinterpret_cast<std::uint32_t (*)(std::uint32_t, std::int32_t)>(
            worker),
        reinterpret_cast<std::uintptr_t>(find_volume_by_name(path)), 0x64,
        reinterpret_cast<std::int32_t>(context));
}

} // namespace

// 006da081 (cdecl). Context {path, -, out, 0, arg}: the binary initializes
// [EBP-0x14] (path), [EBP-0x8] = 0 and [EBP-0x4] = arg and leaves the out
// slot [EBP-0xc] - filled by the worker - plus [EBP-0x10] untouched; after
// the call it copies [EBP-0xc] into *out_result.
void open_file_from_path(const char* path, std::uint32_t* out_result,
                         std::uint32_t arg) {
    std::uint32_t context[5];
    context[0] = reinterpret_cast<std::uint32_t>(path);
    // [EBP-0x10] and [EBP-0xc] stay unwritten exactly as in the binary;
    // [EBP-0xc] doubles as the worker's out slot.
    context[3] = 0;
    context[4] = arg;
    run_file_op(&open_file_reserved, path, context);
    if (out_result != nullptr) {
        *out_result = context[2];
    }
}

// 006da10c (cdecl). Context {path, a, b, 1, c} ([EBP-0x8] = 0x1 via MOV).
void write_file_flagged(const char* path, std::uint32_t a, std::uint32_t b,
                        std::uint32_t c) {
    std::uint32_t context[5];
    context[0] = reinterpret_cast<std::uint32_t>(path);
    context[1] = a;
    context[2] = b;
    context[3] = 1;
    context[4] = c;
    run_file_op(&write_file_reserved, path, context);
}

// 006da14e (cdecl). `AND [EBP-8],0` clears the flag word instead.
void write_file_plain(const char* path, std::uint32_t a, std::uint32_t b,
                      std::uint32_t c) {
    std::uint32_t context[5];
    context[0] = reinterpret_cast<std::uint32_t>(path);
    context[1] = a;
    context[2] = b;
    context[3] = 0;
    context[4] = c;
    run_file_op(&write_file_reserved, path, context);
}

// 006da4b1 (cdecl). Four-word context {path, a, b, 1}; the DEC/NEG/SBB/INC
// tail answers (worker return == 1).
bool check_file_flagged(const char* path, std::uint32_t a, std::uint32_t b) {
    std::uint32_t context[4];
    context[0] = reinterpret_cast<std::uint32_t>(path);
    context[1] = a;
    context[2] = b;
    context[3] = 1;
    return run_file_op(&check_file_reserved, path, context) == 1;
}

// 006da4f3 (cdecl). `AND [EBP-8],0` clears the flag word.
bool check_file_plain(const char* path, std::uint32_t a, std::uint32_t b) {
    std::uint32_t context[4];
    context[0] = reinterpret_cast<std::uint32_t>(path);
    context[1] = a;
    context[2] = b;
    context[3] = 0;
    return run_file_op(&check_file_reserved, path, context) == 1;
}

} // namespace hp6::runtime
