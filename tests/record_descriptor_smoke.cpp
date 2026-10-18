// Local behavior checks for batch strncpy_leaves_058: small record
// zero/construct helpers, unrelated to any particular subsystem observed
// so far.
#include "runtime/RecordDescriptor.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}
} // namespace

int main() {
    using namespace hp6::runtime;

    // ---- clear_quad_word (006b4678) ---------------------------------------
    std::uint32_t quad[4] = {1, 2, 3, 4};
    clear_quad_word(quad);
    expect("quad_cleared", quad[0] == 0 && quad[1] == 0 && quad[2] == 0 && quad[3] == 0);

    // ---- construct_full_record / clear_full_record (006b6833 / 006b6808) --
    alignas(16) std::uint8_t record[0x61];
    std::memset(record, 0xcd, sizeof record);
    std::uint32_t id[4] = {0xaa, 0xbb, 0xcc, 0xdd};
    construct_full_record(record, "hello", 10, 20, 30, 40, id);
    expect("full_name", std::strncmp(reinterpret_cast<char*>(record), "hello", 5) == 0);
    expect("full_name_rest_zeroed", record[5] == 0 && record[0x3f] == 0);
    expect("full_fields",
           *reinterpret_cast<std::uint32_t*>(record + 0x40) == 10 &&
               *reinterpret_cast<std::uint32_t*>(record + 0x44) == 20 &&
               *reinterpret_cast<std::uint32_t*>(record + 0x48) == 30 &&
               *reinterpret_cast<std::uint32_t*>(record + 0x4c) == 40);
    expect("full_id",
           *reinterpret_cast<std::uint32_t*>(record + 0x50) == 0xaa &&
               *reinterpret_cast<std::uint32_t*>(record + 0x5c) == 0xdd);
    expect("full_flag_byte", record[0x60] == 0);

    std::memset(record, 0xcd, sizeof record);
    construct_full_record(record, "x", 1, 2, 3, 4, nullptr);
    expect("full_id_zeroed_when_null", *reinterpret_cast<std::uint32_t*>(record + 0x50) == 0u);

    // ---- construct_named_record / clear_named_record (006b6894 / 006b687f)
    alignas(16) std::uint8_t named_record[0x50];
    std::memset(named_record, 0xcd, sizeof named_record);
    construct_named_record(named_record, "world", id);
    expect("named_name",
           std::strncmp(reinterpret_cast<char*>(named_record), "world", 5) == 0);
    expect("named_name_rest_zeroed", named_record[5] == 0 && named_record[0x3f] == 0);
    expect("named_id",
           *reinterpret_cast<std::uint32_t*>(named_record + 0x40) == 0xaa &&
               *reinterpret_cast<std::uint32_t*>(named_record + 0x4c) == 0xdd);

    std::memset(named_record, 0xcd, sizeof named_record);
    construct_named_record(named_record, "y", nullptr);
    expect("named_id_zeroed_when_null",
           *reinterpret_cast<std::uint32_t*>(named_record + 0x40) == 0u);

    // ---- initialize_object_name_header (006b4c17) -------------------------
    alignas(16) std::uint8_t object[0x500];
    std::memset(object, 0xcd, sizeof object);
    initialize_object_name_header(object, "a_very_long_name_longer_than_31_characters");
    expect("header_name_truncated",
           std::strncmp(reinterpret_cast<char*>(object + 0xb4),
                       "a_very_long_name_longer_than_31", 0x1f) == 0);
    expect("header_fields",
           *reinterpret_cast<std::uint32_t*>(object + 0x4dc) == 0xffffffffu &&
               *reinterpret_cast<std::uint32_t*>(object + 0x4e4) == 0 &&
               *reinterpret_cast<std::uint32_t*>(object + 0x4d8) == 4 &&
               *reinterpret_cast<std::uint32_t*>(object + 0x6c) == 4 &&
               *reinterpret_cast<std::uint32_t*>(object + 0x70) == 9);

    if (failures != 0) {
        std::printf("record_descriptor_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("record_descriptor_smoke: all checks passed\n");
    return 0;
}
