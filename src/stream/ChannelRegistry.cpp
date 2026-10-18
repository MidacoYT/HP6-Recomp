#include "stream/ChannelRegistry.h"
#include "stream/Metadata.h"

// Addresses are read by the binary but absent from the exported globals;
// the final binary must supply their stored values explicitly.
extern std::uint8_t DAT_0089b2fe;  // channel table entry count
extern std::uint32_t DAT_0089b200; // first of up to 255 channel table entries
extern std::uint8_t DAT_0089b438;  // channel system enabled flag

namespace hp6::stream {

// ReAgent candidate: reports/pipeline/candidates/006e0778.cpp.
std::uint32_t channel_table_entry(std::int32_t index) {
    if (index >= 0 && index < static_cast<std::int32_t>(DAT_0089b2fe)) {
        return (&DAT_0089b200)[index];
    }
    return 0;
}

// ReAgent candidate: reports/pipeline/rejected/006e3f71.cpp. ReAgent's
// reviewer rejected the reinterpret_cast chain used to walk the bucket;
// the pointer arithmetic matches the exported x86 (006e3f90-006e3fa6).
std::uint32_t* find_channel_node(std::uint32_t id) {
    if (static_cast<std::int32_t>(id) < 0) return nullptr;
    const auto base = channel_table_entry(static_cast<std::int32_t>(id & 0xffu));
    if (base == 0) return nullptr;
    auto* entry = reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(base));
    auto* node = reinterpret_cast<std::uint32_t*>(
        static_cast<std::uintptr_t>(entry[0x120 / 4]));
    while (node != nullptr && node[3] != id) {
        node = reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(node[0]));
    }
    return node;
}

// ReAgent candidate: reports/pipeline/candidates/006e71e1.cpp.
std::int32_t apply_channel_level(std::int32_t index, std::int32_t level) {
    if (DAT_0089b438 == 0) {
        return static_cast<std::int32_t>(0xfffffff6u);
    }
    const auto base = channel_table_entry(index);
    if (base == 0) {
        return static_cast<std::int32_t>(0xfffffff8u);
    }
    auto* entry = reinterpret_cast<const std::uint32_t*>(static_cast<std::uintptr_t>(base));
    const auto* slot = reinterpret_cast<const std::uintptr_t*>(
        static_cast<std::uintptr_t>(*entry));
    set_header_level_006f4eb9(slot, level);
    return 0;
}

}
