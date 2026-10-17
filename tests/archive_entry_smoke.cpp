// Local behavior checks for the archive entry batch (027). They exercise the
// reconstructed interfaces only; they do not compare against the binary.
#include "archive/Entry.h"
#include "archive/Header.h"

#include <cctype>
#include <cstdio>
#include <cstring>

namespace {

int failures = 0;

void expect(const char* label, bool condition) {
    if (!condition) {
        ++failures;
        std::printf("FAIL %s\n", label);
    }
}

std::uint8_t container[0x40];

void build_type1_container() {
    std::memset(container, 0, sizeof container);
    // 0xC0FB marker, (0x00<<8|0x18)+4 = 0x1c payload offset, no 8-byte tag.
    container[0] = 0xC0u;
    container[1] = 0xFBu;
    container[2] = 0x00u;
    container[3] = 0x18u;
    // Entry 0 at cursor +6: [3-byte big-endian field][name at +0x0c]
    container[0x06] = 0x11u;
    container[0x07] = 0x22u;
    container[0x08] = 0x33u;
    std::memcpy(container + 0x0c, "BEST", 5); // 0x0c..0x10
    // Entry 1: cursor after entry-0 NUL = +0x11; field 0x11..0x13 and the
    // internal double-width name pointer lands at +0x17.
    container[0x11] = 0x44u;
    container[0x12] = 0x55u;
    container[0x13] = 0x66u;
    container[0x14] = 0x77u;
    container[0x15] = 0x88u;
    container[0x16] = 0x99u;
    std::memcpy(container + 0x17, "SHIP", 5); // 0x17..0x1b
}

} // namespace

int main() {
    using namespace hp6::archive;

    expect("cmp_equal", case_insensitive_compare("Best", "bEST") == 0);
    expect("cmp_prefix", case_insensitive_compare("BE", "BEST") != 0);
    expect("cmp_order", case_insensitive_compare("AAA", "AAB") < 0);

    build_type1_container();
    expect("find0_ptr",
           find_entry(container, nullptr, 0, nullptr, nullptr) ==
               container + 0x0c);
    std::int32_t field = -1;
    std::uint32_t raw = 1;
    expect("find1_ptr",
           find_entry(container, nullptr, 1, &field, &raw) ==
               container + 0x17);
    expect("find1_field", field == 0x00445566);
    expect("find1_raw", raw == 0x00778899);
    expect("find1_field_exact", field ==
               static_cast<std::int32_t>(
                   pack_bytes_be(container + 0x11, 3) & 0xffffffffu));
    expect("find1_raw_exact", raw ==
               read_partial_uint(container + 0x14, 3));

    // Name mode is case-insensitive and reaches "BEST" first.
    field = 0;
    expect("find_ptr",
           find_entry(container, "best", 0, &field, nullptr) ==
               container + 0x0c);
    expect("find_field", field == 0x00112233);

    // Miss clears outputs.
    field = 7;
    raw = 8;
    expect("miss", find_entry(container, "MISSING", 0, &field, &raw) == nullptr);
    expect("miss_out", field == 0 && raw == 0);

    std::int32_t value = entry_field_by_name(container, "BEST");
    expect("value32", value == 0x00112233);

    char buffer[8]{};
    expect("copy", copy_entry_string(container, 1, buffer) == buffer);
    expect("copy_content", std::memcmp(buffer, "SHIP", 5) == 0);
    char buffer2[8]{};
    expect("copy_twin", copy_entry_string_twin(container, 0, buffer2) == buffer2);
    expect("copy_twin_content", std::memcmp(buffer2, "BEST", 5) == 0);

    build_type1_container();
    entry_forward(container, "MISS", 0, &field, &raw);
    expect("forward", field == 0 && raw == 0);

    // Tag digits close out the 006dc6a3 twin path.
    std::uint8_t tagged[0x30]{};
    tagged[0] = 0xC0u; tagged[1] = 0xFBu; tagged[2] = 0x00u; tagged[3] = 0x18u;
    tagged[0x14] = 'A'; tagged[0x15] = '5'; tagged[0x16] = '0';
    tagged[0x17] = '3';
    expect("tag_digits", tag_digits_value(tagged) == 503);
    expect("tag_digits_none", tag_digits_value(container) == 0);

    // Secondary 32-bit header field: type 1 reads +4, so container (typed
    // through its zeroed tail) yields 0; the BIGF fixture reads +8.
    expect("field32_t1", header_field_32(container) == 0u);
    const std::uint8_t bigf_field32[]{0x42u, 0x49u, 0x47u, 0x46u, 0, 0, 0, 0,
                                      0x10u, 0x20u, 0x30u, 0x40u, 0, 0, 0, 0};
    expect("field32_bigf", header_field_32(bigf_field32) == 0x10203040u);

    // Name-mode pointer twins (006dc8c2 / 006dc989): pointer equals
    // bytes + low 32 bits of the entry field pack.
    const std::uintptr_t base_ptr =
        reinterpret_cast<std::uintptr_t>(container);
    expect("ptr_by_name",
           reinterpret_cast<std::uintptr_t>(
               entry_pointer_by_name(container, "BEST")) ==
               base_ptr + 0x00112233u);
    expect("ptr_by_name_twin",
           reinterpret_cast<std::uintptr_t>(
               entry_pointer_by_name_twin(container, "best")) ==
               base_ptr + 0x00112233u);
    expect("ptr_by_name_zero_field", entry_pointer_by_name(
               container, "MISSING") == nullptr);

    // Comma-separated name resolution (006dc9ab).
    char names[32];
    std::memcpy(names, "SHIP,NOPE,BEST", 15);
    const std::uint8_t* slots[3]{};
    resolve_entry_list(container, names, slots);
    expect("list0",
           reinterpret_cast<std::uintptr_t>(slots[0]) ==
               base_ptr + 0x00445566u);
    expect("list1", slots[1] == nullptr);
    expect("list2",
           reinterpret_cast<std::uintptr_t>(slots[2]) ==
               base_ptr + 0x00112233u);
    expect("list_names_restored", std::memcmp(names, "SHIP,NOPE,BEST", 15) == 0);

    if (failures != 0) {
        std::printf("archive_entry_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("archive_entry_smoke: all checks passed\n");
    return 0;
}
