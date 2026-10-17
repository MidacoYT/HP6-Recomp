#include "platform/ServiceLifecycle.h"

#include <cstddef>
#include <cstdint>

extern void* DAT_00892df8;
extern void* PTR_LAB_00715d30;
extern void* PTR_FUN_00715d0c;
extern void* PTR_LAB_00715e18;
extern void* PTR_LAB_00715da8;

namespace hp6::platform {
namespace {

#ifdef _MSC_VER
using VirtualCall = void (__thiscall*)(void*);
using ReleaseCall = void (__thiscall*)(void*, void*, std::uint32_t);
#else
using VirtualCall = void (*)(void*);
using ReleaseCall = void (*)(void*, void*, std::uint32_t);
#endif

void invoke_at(std::uint8_t* record, std::size_t record_offset,
               std::size_t table_offset) {
    auto* object = *reinterpret_cast<void**>(record + record_offset);
    auto* table = *reinterpret_cast<std::uintptr_t**>(object);
    reinterpret_cast<VirtualCall>(table[table_offset / 4])(object);
}

} // namespace

void release_block(void* block, std::uint32_t size) {
    auto* allocator = DAT_00892df8;
    auto* table = *reinterpret_cast<std::uintptr_t**>(allocator);
    reinterpret_cast<ReleaseCall>(table[1])(allocator, block, size);
}

void destroy_components(std::uint8_t* record) {
    *reinterpret_cast<void**>(record) = &PTR_LAB_00715e18;
    invoke_at(record, 0x14, 8);
    invoke_at(record, 8, 8);
    invoke_at(record, 0x0c, 8);
    invoke_at(record, 4, 0x0c);
    *reinterpret_cast<void**>(record) = &PTR_LAB_00715da8;
}

void destroy_components_variant(std::uint8_t* record) {
    *reinterpret_cast<void**>(record) = &PTR_LAB_00715d30;
    *reinterpret_cast<void**>(record + 0x1c) = &PTR_FUN_00715d0c;
    destroy_components(record);
}

std::uint8_t* destroy_record_0x84(std::uint8_t* record, std::uint8_t flag) {
    destroy_components_variant(record);
    if ((flag & 1u) != 0) release_block(record, 0x84);
    return record;
}

std::uint8_t* destroy_record_0x1c(std::uint8_t* record, std::uint8_t flag) {
    destroy_components(record);
    if ((flag & 1u) != 0) release_block(record, 0x1c);
    return record;
}

} // namespace hp6::platform
