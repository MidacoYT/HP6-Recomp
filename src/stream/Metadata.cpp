#include "stream/Metadata.h"
#include "runtime/PoolList.h"
#include <bit>

namespace hp6::stream {

// ReAgent candidate: reports/pipeline/candidates/006db3f9.cpp.
// Corrected from the exported MOV [ECX], EAX at 0x006db414: the third output
// is the header address, not the header's 32-bit signature value.
std::uint32_t locate_header(const std::uintptr_t* slot,
                            std::uintptr_t* slot_address,
                            std::uintptr_t* header_address) {
    if (slot != nullptr) {
        const auto* header = reinterpret_cast<const std::uint32_t*>(*slot);
        if (*header == 0x4d525453u) {
            *slot_address = reinterpret_cast<std::uintptr_t>(slot);
            *header_address = reinterpret_cast<std::uintptr_t>(header);
            return 0;
        }
    }
    return 1;
}

// The five initial ReAgent attempts are preserved under reports/pipeline/rejected.
// Exact offsets and choice of slot/header are from the exported x86 assembly.
static std::uint32_t word_at(std::uintptr_t address, std::uintptr_t offset) {
    return *reinterpret_cast<const std::uint32_t*>(address + offset);
}

static void set_word(std::uintptr_t address, std::uintptr_t offset,
                     std::uint32_t value) {
    *reinterpret_cast<std::uint32_t*>(address + offset) = value;
}

std::uint32_t slot_word_04(const std::uintptr_t* slot) {
    std::uintptr_t slot_address = 0, header_address = 0;
    if (locate_header(slot, &slot_address, &header_address) != 0) return 0;
    return word_at(slot_address, 0x04);
}

std::uint32_t slot_word_08(const std::uintptr_t* slot) {
    std::uintptr_t slot_address = 0, header_address = 0;
    if (locate_header(slot, &slot_address, &header_address) != 0) return 0;
    return word_at(slot_address, 0x08);
}

std::uint32_t header_word_44(const std::uintptr_t* slot) {
    std::uintptr_t slot_address = 0, header_address = 0;
    if (locate_header(slot, &slot_address, &header_address) != 0) return 0;
    return word_at(header_address, 0x44);
}

std::int32_t header_delta_40_38(const std::uintptr_t* slot) {
    std::uintptr_t slot_address = 0, header_address = 0;
    if (locate_header(slot, &slot_address, &header_address) != 0) return 0;
    const std::uint32_t bits = word_at(header_address, 0x40) -
                               word_at(header_address, 0x38);
    return std::bit_cast<std::int32_t>(bits);
}

std::uint32_t header_word_58(const std::uintptr_t* slot) {
    std::uintptr_t slot_address = 0, header_address = 0;
    if (locate_header(slot, &slot_address, &header_address) != 0) return 0;
    return word_at(header_address, 0x58);
}

// ReAgent candidate: reports/pipeline/rejected/006db50a.cpp.
// The raw offsets are retained because their member names are not established.
std::uintptr_t find_entry(std::uintptr_t header_address,
                          std::uint32_t identifier) {
    const std::uint32_t index = identifier & 0xffu;
    if (static_cast<std::int32_t>(index) >=
        static_cast<std::int32_t>(word_at(header_address, 0x24))) return 0;
    const std::uintptr_t entry = word_at(header_address, 0x20) + index * 0x128u;
    if (word_at(entry, 0) != identifier || word_at(entry, 4) == 0) return 0;
    return entry;
}

// ReAgent candidate: reports/pipeline/rejected/006dba7f.cpp.
void set_descriptor(const std::uintptr_t* slot, std::int32_t index,
                    std::int32_t first, std::int32_t second,
                    std::int32_t tag) {
    std::uintptr_t slot_address = 0, header_address = 0;
    if (locate_header(slot, &slot_address, &header_address) != 0) return;
    const std::int32_t count =
        static_cast<std::int32_t>(word_at(header_address, 0x2c));
    if (index < 1 || index > count) return;
    if (index == count && (first != 0 || second != 0)) return;
    if (tag < 1 && tag != -1 && tag != -2) return;
    if (tag > static_cast<std::int32_t>(word_at(header_address, 0x34)) ||
        word_at(header_address, 0x44) != 0) return;
    const std::uintptr_t entry = word_at(header_address, 0x28) +
                                  static_cast<std::uint32_t>(index - 1) * 12u;
    set_word(entry, 0, static_cast<std::uint32_t>(first));
    set_word(entry, 4, static_cast<std::uint32_t>(second));
    set_word(entry, 8, static_cast<std::uint32_t>(tag));
}

// ReAgent candidate: reports/pipeline/rejected/006dbaed.cpp.
void set_header_words_48_4c(const std::uintptr_t* slot,
                            std::uint32_t first, std::uint32_t second) {
    std::uintptr_t slot_address = 0, header_address = 0;
    if (locate_header(slot, &slot_address, &header_address) != 0) return;
    set_word(header_address, 0x48, first);
    set_word(header_address, 0x4c, second);
}

// ReAgent candidate: reports/pipeline/rejected/006dbb55.cpp.
std::uintptr_t indexed_record_10(const std::uintptr_t* slot,
                                 std::int32_t index) {
    std::uintptr_t slot_address = 0, header_address = 0;
    if (locate_header(slot, &slot_address, &header_address) != 0) return 0;
    if (index < 1 || index >
        static_cast<std::int32_t>(word_at(header_address, 0x34))) return 0;
    return word_at(header_address, 0x30) +
           static_cast<std::uint32_t>(index - 1) * 16u;
}

// ReAgent candidate: reports/pipeline/rejected/006dbdeb.cpp.
std::uint32_t is_header_inactive(const std::uintptr_t* slot) {
    std::uintptr_t slot_address = 0, header_address = 0;
    if (locate_header(slot, &slot_address, &header_address) != 0) return 0;
    return word_at(header_address, 0x44) == 0 &&
           word_at(slot_address, 0x08) == 0 ? 1u : 0u;
}

// ReAgent candidate: reports/pipeline/rejected/006dbee7.cpp.
std::uint32_t entry_value(const std::uintptr_t* slot,
                          std::uint32_t identifier) {
    std::uintptr_t slot_address = 0, header_address = 0;
    if (locate_header(slot, &slot_address, &header_address) != 0) return 0;
    const std::uintptr_t entry = find_entry(header_address, identifier);
    return entry == 0 ? 0u : word_at(entry, 4);
}

// ReAgent candidates: reports/pipeline/rejected/006dc17c.cpp and
// reports/pipeline/rejected/006dbb17.cpp. Both were rejected by ReAgent's
// own reviewer for using raw struct offsets (already this file's house
// style for this unresolved record) and for an unordered-evaluation
// concern between two independent, side-effect-free reads of header+0x4c
// and header+0x184 that does not change the observable result; see
// architecture/decisions.md, lot stream_priority_051.
void set_header_level(const std::uintptr_t* slot, std::int32_t level) {
    std::uintptr_t slot_address = 0, header_address = 0;
    if (locate_header(slot, &slot_address, &header_address) != 0) return;
    const auto previous = static_cast<std::int32_t>(word_at(header_address, 0x50));
    set_word(header_address, 0x50, static_cast<std::uint32_t>(level));
    const auto threshold = static_cast<std::int32_t>(word_at(header_address, 0x58));
    const auto target = static_cast<std::uint32_t>(threshold < level);
    if (static_cast<std::uint32_t>(threshold < previous) != target) {
        apply_level_target(slot, static_cast<std::int32_t>(target));
    }
}

void apply_level_target(const std::uintptr_t* slot, std::int32_t target) {
    std::uintptr_t slot_address = 0, header_address = 0;
    if (locate_header(slot, &slot_address, &header_address) != 0) return;
    set_word(header_address, 0x54, static_cast<std::uint32_t>(target));
    if (target != 0 && word_at(header_address, 0x44) == 1) {
        hp6::runtime::update_node_priority(
            word_at(header_address, 0x184),
            static_cast<std::uint8_t>(word_at(header_address, 0x4c)));
    }
}

void set_header_level_006f4ffd(const std::uintptr_t* slot, std::int32_t level) {
    set_header_level(slot, level);
}

void set_header_level_006f4eb9(const std::uintptr_t* slot, std::int32_t level) {
    set_header_level_006f4ffd(slot, level);
}

}
