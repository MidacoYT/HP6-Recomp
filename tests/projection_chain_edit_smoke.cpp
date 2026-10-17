#include "projection/AllocationDispatch.h"
#include "projection/RecordChainEdit.h"
#include "projection/RecordLookup.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace hp6::projection {
AllocationDispatch* DAT_00895c84 = nullptr;
}

namespace {
struct Dispatch final : hp6::projection::AllocationDispatch {
    void* released[4]{};
    int count = 0;
    void* call(std::uint32_t, hp6::projection::AllocationArgument*) override {
        return nullptr;
    }
    void release(void* address, std::uint32_t option) override {
        if (option == 0 && count < 4) released[count++] = address;
    }
};
void set_word(std::byte* bytes, std::size_t offset, std::uint32_t value) {
    std::memcpy(bytes + offset, &value, 4);
}
std::uint32_t word(std::byte* bytes, std::size_t offset) {
    std::uint32_t value;
    std::memcpy(&value, bytes + offset, 4);
    return value;
}
}

int main() {
    alignas(16) std::byte records[0x80]{};
    auto* first = records;
    auto* second = records + 0x20;
    auto* added = records + 0x40;
    set_word(first, 0, 0x86f); // kind 0x20, owns its relative payload
    set_word(first, 4, 0x20);
    set_word(first, 8, 0x10);
    set_word(second, 0, 0xc69); // kind 0x80, owns payload and record
    set_word(second, 8, 0x10);
    set_word(added, 0, 0x6f);
    Dispatch dispatch;
    hp6::projection::DAT_00895c84 = &dispatch;
    using namespace hp6::projection;
    if (insert_record_after_kind(first, added) != 1 ||
        word(first, 4) != 0x40 || word(added, 4) != 0xffffffe0u)
        return 1;
    if (remove_record_by_kind(first, 0x80) != 1 ||
        word(added, 4) != 0 || dispatch.count != 2 ||
        dispatch.released[0] != second + 0x10 ||
        dispatch.released[1] != second) return 2;
    if (remove_record_by_kind(first, 0x20) != 1 ||
        static_cast<unsigned>(first[0]) != 0x81 ||
        dispatch.count != 3 || dispatch.released[2] != first + 0x10 ||
        find_variant_20(first) != added) return 3;
    if (remove_record_by_kind(first, 4) != 0) return 4;
    return 0;
}
