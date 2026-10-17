#include "projection/RecordChainEdit.h"
#include "projection/AllocationDispatch.h"
#include "projection/RecordDecode.h"
#include "projection/RecordFields.h"
#include <cstddef>
#include <cstring>

namespace hp6::projection {
namespace {
std::uint32_t word(const void* record, std::size_t offset) {
    std::uint32_t value;
    std::memcpy(&value, static_cast<const std::byte*>(record) + offset, 4);
    return value;
}

std::byte* relative(std::byte* record, std::size_t offset) {
    const auto distance = word(record, offset);
    return distance == 0 ? nullptr : record + static_cast<std::int32_t>(distance);
}
}

void release_record_chain(void* first) {
    auto* node = static_cast<std::byte*>(first);
    auto* next = relative(node, 4);
    if (next != nullptr) release_record_chain(next);
    const auto flags = word(node, 0);
    if (flags & 0x800u) release_projection_memory(relative(node, 8));
    if (flags & 0x400u) release_projection_memory(node);
}

std::uint8_t remove_record_by_kind(void* first, std::uint32_t kind) {
    auto* node = static_cast<std::byte*>(first);
    std::byte* previous = nullptr;
    while (node != nullptr && classify_record(node) != kind) {
        previous = node;
        node = relative(node, 4);
    }
    if (node == nullptr) return 0;
    if (previous == nullptr) {
        node[0] = std::byte{0x81};
        if (word(node, 0) & 0x800u) {
            release_projection_memory(relative(node, 8));
        }
    } else {
        link_next_record(previous, relative(node, 4));
        release_record_chain(node);
    }
    return 1;
}

std::uint8_t insert_record_after_kind(void* first, void* incoming) {
    auto* node = static_cast<std::byte*>(first);
    auto* added = static_cast<std::byte*>(incoming);
    const auto desired = classify_record(added);
    std::byte* previous = nullptr;
    for (;;) {
        if (node == nullptr) {
            link_next_record(previous, added);
            link_next_record(added, nullptr);
            return 1;
        }
        const auto current = classify_record(node);
        auto* next = relative(node, 4);
        if (current == desired) {
            link_next_record(added, next);
            link_next_record(node, added);
            return 1;
        }
        previous = node;
        node = next;
    }
}

}
