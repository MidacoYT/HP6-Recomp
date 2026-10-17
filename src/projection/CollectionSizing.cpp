#include "projection/CollectionSizing.h"
#include "projection/FormatSize.h"
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
}

std::uint32_t measure_collection_update(const void* first,
                                        const void* input,
                                        std::int32_t operation) {
    std::uint32_t pending = word(input, 4);
    std::uint32_t size = 0;
    bool previous = false;
    auto* node = static_cast<const std::byte*>(first);
    for (;;) {
        if (node == nullptr) {
            if (operation == 1 || operation == 3) {
                if (pending & 2u) {
                    size = static_cast<std::uint32_t>(estimate_record_size(input)) -
                           0x20u + ((size + 0x7fu) & ~0x7fu);
                }
                if (pending & 4u) {
                    size = ((size + 0x0fu) & ~0x0fu) +
                           static_cast<std::uint32_t>(estimate_variant_04_size(input));
                }
                if (pending & 0x100u) {
                    size = ((size + 0x0fu) & ~0x0fu) + fixed_clip_storage_size();
                }
                if (pending & 0x10u) {
                    size = ((size + 0x0fu) & ~0x0fu) +
                           static_cast<std::uint32_t>(estimate_variant_10_size(input));
                }
                if (pending & 0x80u) {
                    size = eagl_storage_size(input) + 0x10u +
                           ((size - 1u) & ~0x0fu);
                }
                if (pending & 0x20u) {
                    size = comment_storage_size(input) + 0x10u +
                           ((size - 1u) & ~0x0fu);
                }
            }
            return size;
        }
        const auto kind = classify_record(node);
        if (kind == 2) {
            if ((pending & 2u) == 0 || operation != 2) {
                if (previous) size = ((size + 0x7fu) & ~0x7fu) - 0x20u;
                if ((pending & 2u) == 0 || operation != 3)
                    size += 0x20u + word(node, 0x0c);
                else size += static_cast<std::uint32_t>(estimate_record_size(input));
            }
            pending &= ~2u;
        } else if (kind == 4) {
            if ((pending & 4u) == 0 || operation != 2) {
                if (previous) size = (size + 0x0fu) & ~0x0fu;
                if ((pending & 4u) == 0 || operation != 3)
                    size += 0x20u + word(node, 0x0c);
                else size += static_cast<std::uint32_t>(estimate_variant_04_size(input));
            }
            pending &= ~4u;
        } else if (kind == 0x10) {
            if ((pending & 0x10u) == 0 || operation != 2) {
                if (previous) size = (size + 0x0fu) & ~0x0fu;
                if ((pending & 0x10u) == 0 || operation != 3)
                    size += 0x20u + word(node, 0x0c);
                else size += static_cast<std::uint32_t>(estimate_variant_10_size(input));
            }
            pending &= ~0x10u;
        } else if (kind == 0x20) {
            if ((pending & 0x20u) == 0 || operation != 2) {
                if (previous) size = ((size - 1u) & ~0x0fu) + 0x10u;
                if ((pending & 0x20u) == 0 || operation != 3)
                    size += 0x10u + word(node, 0x0c);
                else size += comment_storage_size(input);
            }
            pending &= ~0x20u;
        } else if (kind == 0x80) {
            if ((pending & 0x80u) == 0 || operation != 2) {
                if (previous) size = ((size - 1u) & ~0x0fu) + 0x10u;
                if ((pending & 0x80u) == 0 || operation != 3)
                    size += 0x10u + word(node, 0x0c);
                else size += eagl_storage_size(input);
            }
            pending &= ~0x80u;
        } else if (kind == 0x100) {
            // This branch is present in the caller but the currently observed
            // classifier returns only values fitting in one byte.
            if ((pending & 0x100u) == 0 || operation != 2) {
                if (previous) size = (size + 0x0fu) & ~0x0fu;
                if ((pending & 0x100u) == 0 || operation != 3)
                    size += record_storage_size(node);
                else size += fixed_clip_storage_size();
            }
            pending &= ~0x100u;
        } else if (kind != 1) {
            return 0;
        }
        previous = true;
        const auto offset = word(node, 4);
        node = offset == 0 ? nullptr : node + static_cast<std::int32_t>(offset);
    }
}

std::uint32_t measure_record_replacement(const void* first,
                                         const void* input,
                                         std::int32_t operation) {
    const auto base = reinterpret_cast<std::uintptr_t>(first);
    const std::uint32_t offset16 = static_cast<std::uint32_t>(
        ((base + 0x0fu) & ~std::uintptr_t{0x0f}) - base);
    const std::uint32_t offset128 = static_cast<std::uint32_t>(
        ((base + 0x7fu) & ~std::uintptr_t{0x7f}) - base);
    const auto align_after = [offset16, offset128](std::uint32_t total,
                                                    std::uint32_t kind) {
        if (kind == 4)
            return (((total - offset16 + 0x2fu) & ~0x0fu) - 0x20u) + offset16;
        return ((total - offset128 + 0x0fu) & ~0x0fu) + offset128;
    };
    const auto desired = classify_record(input);
    std::uint32_t total = 0;
    std::uint32_t last_kind = 0;
    bool found = false;
    for (auto* node = static_cast<const std::byte*>(first); node != nullptr;) {
        const auto kind = classify_record(node);
        last_kind = kind;
        std::uint32_t current_size = 0;
        if (kind != desired || operation == 1) {
            current_size = record_storage_size(node);
            if (total != 0) total = align_after(total, kind);
        }
        total += current_size;
        if (kind == desired && !found) {
            found = true;
            if (operation == 3 || operation == 1) {
                const auto replacement_size = record_storage_size(input);
                if (total != 0) total = align_after(total, kind);
                total += replacement_size;
            }
        }
        const auto offset = word(node, 4);
        node = offset == 0 ? nullptr : node + static_cast<std::int32_t>(offset);
    }
    if (found) return total;
    if (operation != 2) {
        if (total != 0) total = align_after(total, last_kind);
        total += record_storage_size(input);
    }
    return total;
}

}
