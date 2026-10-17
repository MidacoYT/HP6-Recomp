#include "runtime/PoolPath.h"

#include <cstring>

#include "runtime/PoolAccess.h"
#include "runtime/PoolList.h"
#include "runtime/VolumeRegistry.h"

// Error-report object pointer (runtime-filled, exported as data without
// its value) and the static literals used for the null-path fallback and
// the source-file record.
extern void* DAT_00851384;
extern const char empty_string_007278d3[1];
extern const char node_source_path_00715a30[96];
// Named-node vtables of the binary's data space; tests supply inert
// definitions.
extern void* PTR_FUN_00715a90;
extern void* PTR_FUN_00715aa0;
extern void* PTR_FUN_00715ab0;
// Service root global (also referenced by VolumeRegistry).
extern std::int32_t DAT_00892b60;

namespace hp6::runtime {

// 006b3b04 resolves through the CRT runtime dispatch handled by std::memcpy.
void attach_node_path(std::uint8_t* node, const char* path) {
    const char* source = path;
    if (source == nullptr) {
        source = empty_string_007278d3;
    }
    // The binary walks the string inline; std::strlen is equivalent.
    const std::uint32_t copy_length =
        static_cast<std::uint32_t>(std::strlen(source)) + 1;

    // Twelve dwords, exactly the stack block [EBP-0x30..-0x4] of the
    // exported x86.
    std::uint32_t record[12] = {};
    record[0] = 6;                                   // [EBP-0x30]
    record[1] = 0x1cc;                               // [EBP-0x2c] line 460
    record[2] = reinterpret_cast<std::uint32_t>(&record[3]);
    record[3] = 5;                                   // [EBP-0x24]
    record[4] = reinterpret_cast<std::uint32_t>(node_source_path_00715a30);
    record[5] = reinterpret_cast<std::uint32_t>(&record[6]);
    record[6] = 1;                                   // [EBP-0x18]
    record[7] = reinterpret_cast<std::uint32_t>(source);  // [EBP-0x14]
    record[8] = reinterpret_cast<std::uint32_t>(&record[9]);
    record[9] = 4;                                   // [EBP-0xc]
    record[10] = 1;                                  // [EBP-0x8]
    record[11] = 0;                                  // [EBP-0x4]

    auto* object =
        reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(DAT_00851384));
    const auto vtable =
        static_cast<std::uintptr_t>(*(reinterpret_cast<const std::uint32_t*>(object)));
#ifdef _MSC_VER
    // thiscall, object in ECX, two stack arguments cleaned by the callee.
    using AllocateCall = void* (__thiscall*)(void* self, std::uint32_t length,
                                             void* request);
#else
    using AllocateCall = void* (*)(void* self, std::uint32_t length,
                                   void* request);
#endif
    const auto allocate = reinterpret_cast<AllocateCall>(
        *reinterpret_cast<void**>(vtable));
    void* buffer = allocate(object, copy_length,
                            reinterpret_cast<void*>(record));
    *reinterpret_cast<void**>(node + 0x30) = buffer;
    std::memcpy(buffer, source, copy_length);
}

namespace {

std::uint8_t* construct_named_node(std::uint8_t* receiver, const char* path,
                                   std::uint8_t mode,
                                   std::uint32_t attach_words,
                                   std::uintptr_t engine, void* vtable) {
    attach_engine(receiver, mode, attach_words, engine);
    *reinterpret_cast<void**>(receiver) = vtable;
    attach_node_path(receiver, path);
    schedule_node(receiver);
    return receiver;
}

} // namespace

std::uint8_t* construct_named_node_00715a90(std::uint8_t* receiver,
                                            const char* path,
                                            std::uint8_t mode,
                                            std::uint32_t attach_words,
                                            std::uintptr_t engine) {
    return construct_named_node(receiver, path, mode, attach_words, engine,
                                &PTR_FUN_00715a90);
}

std::uint8_t* construct_named_node_00715aa0(std::uint8_t* receiver,
                                            const char* path,
                                            std::uint8_t mode,
                                            std::uint32_t attach_words,
                                            std::uintptr_t engine) {
    return construct_named_node(receiver, path, mode, attach_words, engine,
                                &PTR_FUN_00715aa0);
}

// Bounded word writer of 006b2fbd: attach, vtable 00715ab0, path
// attachment, bounded words (+0x24 then +0x20), scheduling last.
std::uint8_t* construct_bounded_path_node(
    std::uint8_t* receiver, const char* path, std::uint32_t field_0x20,
    std::uint8_t mode, std::uint32_t attach_words, std::uintptr_t engine) {
    attach_engine(receiver, mode, attach_words, engine);
    *reinterpret_cast<void**>(receiver) = &PTR_FUN_00715ab0;
    attach_node_path(receiver, path);
    *reinterpret_cast<std::uint32_t*>(receiver + 0x24) = 0;
    *reinterpret_cast<std::uint32_t*>(receiver + 0x20) = field_0x20;
    schedule_node(receiver);
    return receiver;
}

// Raw word/pointer helpers over the game address space.
std::uint8_t* pointer(std::uint32_t value) {
    return reinterpret_cast<std::uint8_t*>(value);
}

std::uint32_t word(const std::uint8_t* bytes, std::size_t offset) {
    return *reinterpret_cast<const std::uint32_t*>(bytes + offset);
}

std::uint32_t address(const std::uint8_t* bytes) {
    return static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(bytes));
}

// Volume name getter without the optional stack argument: thiscall with
// the object in ECX only (exported x86 of 006b28d4: MOV ECX,[EBX+0x84];
// MOV EAX,[ECX]; CALL [EAX+0x30]; no push). See the VolumeNameNoArgCall
// comment for the explicit-argument modeling.
// Volume name getter of the record's object: thiscall with the object in
// ECX. The same vtable slot +0x30 is observed both with one stack
// argument (006b25b8, callee-cleaned) and with none (006b28d4 ECX-only
// call); because the callee always cleans one stack word (RET 4), the
// ECX-only call in the binary silently consumes four bytes of the
// caller's frame (EBP-relative, so benign there). The reconstruction
// models the call with an explicit argument so the generated stack stays
// balanced; the stubs ignore the value.
#ifdef _MSC_VER
using VolumeNameNoArgCall = char* (__thiscall*)(void* object, void* scratch);
#else
using VolumeNameNoArgCall = char* (*)(void* object, void* scratch);
#endif

std::uint32_t create_named_node_a_from_record(const char* path,
                                              std::uint8_t mode,
                                              std::uint32_t attach_words) {
    std::uint8_t* slot = detach_first_locked(0x40);
    const auto engine = reinterpret_cast<std::uintptr_t>(
        find_volume_by_name(path));
    construct_named_node_00715a90(slot, path, mode, attach_words, engine);
    return *reinterpret_cast<std::uint32_t*>(slot + 0xc);
}

std::uint32_t create_bounded_path_node_from_record(
    const char* path, std::uint32_t field_0x20, std::uint8_t mode,
    std::uint32_t attach_words) {
    std::uint8_t* slot = detach_first_locked(0x40);
    const auto engine = reinterpret_cast<std::uintptr_t>(
        find_volume_by_name(path));
    construct_bounded_path_node(slot, path, field_0x20, mode, attach_words,
                                engine);
    return *reinterpret_cast<std::uint32_t*>(slot + 0xc);
}

std::uint32_t create_named_node_b_from_volume(const char* path,
                                              std::uint8_t mode,
                                              std::uint32_t attach_words) {
    std::uint8_t* volume;
    if (std::strchr(path, ':') == nullptr) {
        const auto root = pointer(static_cast<std::uint32_t>(DAT_00892b60));
        volume = find_volume_by_name(
            *reinterpret_cast<char* const*>(word(root, 0x88)));
        if (*path != '\\' || path[1] != '\\') {
            auto* object = pointer(word(volume, 0x84));
            const auto vtable =
                static_cast<std::uintptr_t>(word(object, 0));
            const auto name_of = reinterpret_cast<VolumeNameNoArgCall>(
                *reinterpret_cast<void**>(vtable + 0x30));
            const char* name = name_of(object, nullptr);
            // 260-byte stack buffer ([EBP-0x104] in the x86): the binary
            // copies the name with its NUL, then appends the path over the
            // name terminator with its own NUL (inline strlen + REP
            // MOVSD) - strcpy + strcat semantics on unzeroed storage.
            char full_path[260];
            std::strcpy(full_path, name);
            std::strcat(full_path, path);
            path = full_path;
        }
    } else {
        volume = find_volume_by_name(path);
    }
    std::uint8_t* slot = detach_first_locked(0x40);
    construct_named_node_00715aa0(slot, path, mode, attach_words,
                                  reinterpret_cast<std::uintptr_t>(volume));
    return *reinterpret_cast<std::uint32_t*>(slot + 0xc);
}

// 006b1a59 exports: the path entry count and the path buffer capacity.
extern "C" std::int32_t DAT_00851390;
extern "C" std::int32_t DAT_00851394;

// 006b9386 resolves through the CRT runtime dispatch handled by
// std::strchr; 006b3b0e is the integrated clear_buffer zeroing helper.
void set_game_search_paths(const char* list) {
    auto* root = pointer(static_cast<std::uint32_t>(DAT_00892b60));
    // The binary clears the array (DAT_00851390 << 2 bytes, 006b3b0e) and
    // the whole path buffer (DAT_00851394 bytes, 006b3b0e) first.
    clear_buffer(word(root, 0x88),
                 static_cast<std::uint32_t>(DAT_00851390) << 2);
    clear_buffer(word(root, 0x8c),
                 static_cast<std::uint32_t>(DAT_00851394));
    auto* buffer = reinterpret_cast<char*>(pointer(word(root, 0x8c)));
    auto* write = buffer;
    for (const char* read = list; (*write++ = *read++) != '\0';) {}
    auto* table = pointer(word(root, 0x88));
    std::uint32_t entries = 1;
    *reinterpret_cast<std::uint32_t*>(table) = address(reinterpret_cast<const std::uint8_t*>(buffer));
    char* scan = buffer;
    for (;;) {
        auto* found = std::strchr(scan, ';');
        if (found == nullptr) {
            break;
        }
        *found = '\0';
        if (found[-1] == '/' || found[-1] == '\\') {
            found[-1] = '\0';
        }
        scan = found + 1;
        *reinterpret_cast<std::uint32_t*>(table + entries * 4) =
            address(reinterpret_cast<const std::uint8_t*>(scan));
       ++entries;
    }
    // The final entry receives the trailing-slash treatment as well.
    auto* last = reinterpret_cast<char*>(pointer(word(table, (entries - 1) * 4)));
    auto* end = last;
    while (*end != '\0') {
        ++end;
    }
    if (end[-1] == '/' || end[-1] == '\\') {
        end[-1] = '\0';
    }
}

} // namespace hp6::runtime
