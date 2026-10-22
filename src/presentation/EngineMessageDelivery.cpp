#include <cstdint>

extern std::int32_t DAT_00d63e80;
// 0x00FF87D0 removes listeners deferred during notification.
extern void flush_deferred_listeners_00ff87d0(void* header);

namespace {
#ifdef _MSC_VER
using NotifyCall = void (__thiscall*)(void*, std::int32_t*);
using DeleteCall = void (__thiscall*)(void*, std::uint32_t);
#else
using NotifyCall = void (*)(void*, std::int32_t*);
using DeleteCall = void (*)(void*, std::uint32_t);
#endif

void* pointer_from_word(std::int32_t word) {
    return reinterpret_cast<void*>(
        static_cast<std::uintptr_t>(static_cast<std::uint32_t>(word)));
}

void* first_method(void* object, std::uint32_t offset) {
    return *reinterpret_cast<void**>(
        *reinterpret_cast<std::uint8_t**>(object) + offset);
}
} // namespace

std::int32_t deliver_tick_message_005db240(std::int32_t* handle) {
    // 0x005DB240..0x005DB2AF. The list head is itself a 0x14-byte record;
    // offset +0x12 is a deferred-removal flag, +0x13 is "dispatch active".
    auto* header = static_cast<std::uint8_t*>(pointer_from_word(*handle));
    if (*reinterpret_cast<void**>(header) == header) return 0;
    header[0x13] = 1;
    for (auto* node = *reinterpret_cast<std::uint8_t**>(header);
         node != header;
         node = *reinterpret_cast<std::uint8_t**>(node)) {
        auto* listener = *reinterpret_cast<void**>(node + 8);
        if (listener != nullptr) {
            reinterpret_cast<NotifyCall>(first_method(listener, 4))(listener, handle);
        }
    }
    header[0x13] = 0;
    if (header[0x12] != 0) {
        flush_deferred_listeners_00ff87d0(header);
        header[0x12] = 0;
    }
    return 1;
}

void deliver_delete_entity_005db160(const std::uint32_t* record) {
    // 0x005DB160..0x005DB191 receives record in EAX in the x86 image.
    auto* header = reinterpret_cast<std::uint8_t*>(
        static_cast<std::uintptr_t>(record[0]));
    if (header == pointer_from_word(DAT_00d63e80)) return;
    while (auto* node = *reinterpret_cast<std::uint8_t**>(header)) {
        if (node == header) break;
        auto* listener = *reinterpret_cast<void**>(node + 8);
        if (listener != nullptr) {
            reinterpret_cast<DeleteCall>(first_method(listener, 0))(listener, 1);
        }
    }
}
