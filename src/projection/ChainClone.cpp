#include "projection/ChainClone.h"
#include "projection/AllocationDispatch.h"
#include "projection/FormatSize.h"
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

const std::byte* next_record(const void* record) {
    const auto offset = word(record, 4);
    return offset == 0 ? nullptr :
        static_cast<const std::byte*>(record) + static_cast<std::int32_t>(offset);
}

std::uintptr_t aligned_node(std::uintptr_t cursor, std::uint32_t kind) {
    if (kind == 2) return ((cursor + 0x9fu) & ~std::uintptr_t{0x7f}) - 0x20u;
    if (kind == 4 || kind == 0x10 || kind == 0x100)
        return ((cursor + 0x2fu) & ~std::uintptr_t{0x0f}) - 0x20u;
    return ((cursor + 0x1fu) & ~std::uintptr_t{0x0f}) - 0x10u;
}

}

std::uint32_t measure_clone_chain(const void* first) {
    std::uint32_t total = 0;
    bool previous = false;
    for (auto* record = static_cast<const std::byte*>(first); record != nullptr;
         record = next_record(record)) {
        const auto kind = classify_record(record);
        if (kind == 2) {
            if (previous) total = ((total + 0x7fu) & ~0x7fu) - 0x20u;
            total += 0x20u + word(record, 0x0c);
        } else if (kind == 4 || kind == 0x10) {
            if (previous) total = (total + 0x0fu) & ~0x0fu;
            total += 0x20u + word(record, 0x0c);
        } else if (kind == 0x20 || kind == 0x80) {
            if (previous) total = ((total - 1u) & ~0x0fu) + 0x10u;
            total += 0x10u + word(record, 0x0c);
        } else if (kind == 0x100) {
            if (previous) total = (total + 0x0fu) & ~0x0fu;
            total += record_storage_size(record);
        } else if (kind != 1) {
            return 0;
        }
        previous = true;
    }
    return total;
}

void clone_record_chain_into(const void* first, void* storage) {
    auto* source = static_cast<const std::byte*>(first);
    void* previous = nullptr;
    std::uintptr_t cursor = reinterpret_cast<std::uintptr_t>(storage);
    for (;;) {
        if (source == nullptr) {
            // The x86 routine terminates the final link without a guard.
            link_next_record(previous, nullptr);
            return;
        }
        const auto kind = classify_record(source);
        void* output = previous;
        if (kind == 2 || kind == 4 || kind == 0x10 || kind == 0x20 ||
            kind == 0x80 || kind == 0x100) {
            output = reinterpret_cast<void*>(aligned_node(cursor, kind));
            if (previous != nullptr) link_next_record(previous, output);
            switch (kind) {
            case 2: clone_variant_02(source, output); break;
            case 4: clone_variant_04(source, output); break;
            case 0x10: clone_variant_10(source, output); break;
            case 0x20: clone_variant_20(source, output); break;
            case 0x80: clone_variant_80(source, output); break;
            case 0x100: clone_variant_100(source, output); break;
            }
            cursor = reinterpret_cast<std::uintptr_t>(output) +
                (kind == 0x100 ? record_storage_size(output) :
                 word(output, 0x0c) + (kind == 0x20 || kind == 0x80 ? 0x10u : 0x20u));
        }
        previous = output;
        source = next_record(source);
    }
}

void* clone_record_chain_allocated(const void* first, std::uint32_t context) {
    auto* result = dispatch_allocation("cl_shp", measure_clone_chain(first),
                                       0x80, 0x20, context, "shpclone.cpp", 0x1c4);
    clone_record_chain_into(first, result);
    std::uint32_t flags = word(result, 0);
    flags = (flags & ~0x400u) | 0x400u;
    std::memcpy(result, &flags, 4);
    return result;
}

}
