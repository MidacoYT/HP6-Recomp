#include "projection/RecordLookup.h"
#include "projection/RecordFields.h"
#include <cstddef>
#include <cstring>

namespace hp6::projection {

void* find_record_by_kind(void* first, std::uint32_t kind) {
    auto* node = static_cast<std::byte*>(first);
    while (node != nullptr) {
        if (classify_record(node) == kind) return node;
        std::uint32_t offset;
        std::memcpy(&offset, node + 4, 4);
        node = offset == 0 ? nullptr : node + static_cast<std::int32_t>(offset);
    }
    return nullptr;
}

void* find_variant_02(void* first) { return find_record_by_kind(first, 2); }
void* find_variant_04(void* first) { return find_record_by_kind(first, 4); }
void* find_variant_10(void* first) { return find_record_by_kind(first, 0x10); }
void* find_variant_20(void* first) { return find_record_by_kind(first, 0x20); }

}
