#pragma once

#include <cstdint>

namespace hp6::runtime {

// 006b1968 (thiscall in the binary; node as first argument). Resolves
// `path` (null falls back to the empty string at 0x007278d3), computes
// its length, builds the loader error record on the stack (type 6, line
// 0x1cc = 460, sub-record id 5 at +0x24 pointing at the source literal
// 0x00715a30 "filesys.cpp", path word +0x14, length word +0xc, flag
// words 1/1), asks the error-report object at DAT_00851384 (vtable slot
// +0, thiscall, callee-cleaned stack arguments: length then record) for
// a buffer of length + 1, stores it in node + 0x30 and copies the path
// with its NUL through the CRT memcpy dispatch (006b3b04).
void attach_node_path(std::uint8_t* node, const char* path);

// 006b2ef3 (thiscall in the binary): attach (006b20e8), vtable
// 00715a90, path attachment (006b1968), scheduling (006b2199).
std::uint8_t* construct_named_node_00715a90(std::uint8_t* receiver,
                                            const char* path,
                                            std::uint8_t mode,
                                            std::uint32_t attach_words,
                                            std::uintptr_t engine);

// 006b2f40 (thiscall in the binary): identical writer shape with vtable
// 00715aa0.
std::uint8_t* construct_named_node_00715aa0(std::uint8_t* receiver,
                                            const char* path,
                                            std::uint8_t mode,
                                            std::uint32_t attach_words,
                                            std::uintptr_t engine);

// 006b2fbd (thiscall in the binary): attach, vtable 00715ab0, path
// attachment (006b1968), bounded words (+0x24 = 0, +0x20 = param_2), then
// scheduling (006b2199).
std::uint8_t* construct_bounded_path_node(std::uint8_t* receiver,
                                          const char* path,
                                          std::uint32_t field_0x20,
                                          std::uint8_t mode,
                                          std::uint32_t attach_words,
                                          std::uintptr_t engine);

// 006b289e (cdecl): 0x40 slot pull, engine resolved by find_volume_by_name
// over the path itself (may be null), then 006b3223... 006b2ef3 named node
// A. The null slot path faults at [null+0xc] exactly like the binary.
std::uint32_t create_named_node_a_from_record(const char* path,
                                              std::uint8_t mode,
                                              std::uint32_t attach_words);

// 006b2990 (cdecl): 0x40 slot pull, engine = find_volume_by_name(path),
// then 006b2fbd. Null slot path faults like the binary.
std::uint32_t create_bounded_path_node_from_record(const char* path,
                                                   std::uint32_t field_0x20,
                                                   std::uint8_t mode,
                                                   std::uint32_t attach_words);

// 006b28d4 (cdecl): named node B (006b2f40) with a volume-aware path.
// Without ':' the engine is looked up over the game path (word at root
// + 0x88, one indirection); unless the path starts with \\\\ the volume
// name is consulted (+vtable slot +0x30 thiscall, no stack argument) and
// a 260-byte stack buffer is filled with name + separator + path
// (strcpy of the name with its NUL, then the path copied - including its
// NUL - over the name terminator, i.e. strcpy+strcat semantics), and that
// buffer becomes the stored path. With ':' the engine comes from the
// path itself and the path is stored as given.
std::uint32_t create_named_node_b_from_volume(const char* path,
                                              std::uint8_t mode,
                                              std::uint32_t attach_words);

// 006b1a59 (cdecl): installs the ';'-separated search path list. The
// string is copied into the buffer pointed at by root + 0x8c (capacity
// DAT_00851394 bytes, full-strength memcpy of the array at root + 0x88
// with DAT_00851390 << 2 bytes first), array[0] points at the buffer, and
// each ';' found by the CRT strchr thunk (006b9386) becomes a terminator:
// a '/' or '\\' before it is dropped, and the following slot gets the
// next token. After the last ';' the final entry receives the same
// trailing-slash treatment. The entry count globals are read, never
// written.
void set_game_search_paths(const char* list);

} // namespace hp6::runtime
