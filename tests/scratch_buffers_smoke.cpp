// Local behavior checks for batch mixed_deps_053. Game-space symbols get
// test-local definitions; _DAT_00000000 is an ordinary global here (the
// reconstruction cannot place a variable at the literal address 0), so the
// fatal out-of-memory trap of bump_allocate is exercised directly rather
// than avoided.
#include "runtime/ScratchBuffers.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

void* DAT_00fcade0 = nullptr;
std::int32_t* DAT_00cf18e4 = nullptr;
volatile std::uint32_t _DAT_00000000 = 0;
std::byte _Src_00c754d0[16] = {};
std::int32_t DAT_00cf19a0 = 0;
std::uint32_t* DAT_00cf1a08 = nullptr;
std::uint32_t* DAT_00cf1920 = nullptr;
std::int32_t DAT_00866368 = 0;
std::uint32_t DAT_00cf1918 = 0;
std::uint32_t DAT_00cf191c = 0;
std::byte _Dst_00bb0a70[32] = {};
std::byte _Dst_0096b490[32] = {};

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}
std::uint32_t address(const void* value) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}
} // namespace

int main() {
    using namespace hp6::runtime;
    static_assert(sizeof(void*) == 4, "The binary stores 32-bit addresses");

    // ---- store_payload (00422e20) ---------------------------------------
    alignas(16) std::uint8_t self_buf[0x820];
    std::memset(self_buf, 0, sizeof self_buf);
    const char small_data[4] = {1, 2, 3, 4};
    store_payload(self_buf, small_data, 4);
    expect("store_small_data", std::memcmp(self_buf + 0x10, small_data, 4) == 0);
    expect("store_small_dest", *reinterpret_cast<void**>(self_buf + 0x810) ==
                               self_buf + 0x10);
    expect("store_small_flag", self_buf[8] == 1);

    alignas(16) std::uint8_t shared[0x2400];
    alignas(16) std::uint8_t shared_descriptor[0x20];
    std::memset(shared_descriptor, 0, sizeof shared_descriptor);
    *reinterpret_cast<void**>(shared_descriptor + 0x18) = shared;
    DAT_00fcade0 = shared_descriptor;

    std::memset(self_buf, 0, sizeof self_buf);
    static char big_data[0x900];
    std::memset(big_data, 0x5a, sizeof big_data);
    store_payload(self_buf, big_data, 0x900);
    expect("store_large_data", std::memcmp(shared, big_data, 0x900) == 0);
    expect("store_large_dest", *reinterpret_cast<void**>(self_buf + 0x810) ==
                               static_cast<void*>(shared));
    expect("store_large_flag", self_buf[8] == 1);

    std::memset(self_buf, 0, sizeof self_buf);
    store_payload(self_buf, big_data, 0x2400);
    expect("store_too_large_noop", self_buf[8] == 0);

    *reinterpret_cast<void**>(shared_descriptor + 0x18) = nullptr;
    std::memset(self_buf, 0, sizeof self_buf);
    store_payload(self_buf, big_data, 0x900);
    expect("store_missing_shared_noop", self_buf[8] == 0);

    // ---- bump_allocate (0063b850) ----------------------------------------
    alignas(16) std::uint8_t arena_storage[0x100];
    std::int32_t arena[3] = {
        static_cast<std::int32_t>(address(arena_storage)), 0x100,
        static_cast<std::int32_t>(address(arena_storage))};
    DAT_00cf18e4 = arena;

    const auto first = bump_allocate(0xdeadu, 8);
    expect("bump_first", first == address(arena_storage));
    expect("bump_first_advance",
           arena[2] == static_cast<std::int32_t>(address(arena_storage)) + 8);

    const auto second = bump_allocate(0, 5); // rounds up to 8
    expect("bump_second", second == address(arena_storage) + 8);
    expect("bump_second_advance",
           arena[2] == static_cast<std::int32_t>(address(arena_storage)) + 16);
    expect("bump_no_trap_yet", _DAT_00000000 == 0);

    alignas(16) std::uint8_t tiny_storage[4];
    std::int32_t tiny_arena[3] = {static_cast<std::int32_t>(address(tiny_storage)),
                                  4,
                                  static_cast<std::int32_t>(address(tiny_storage))};
    DAT_00cf18e4 = tiny_arena;
    _DAT_00000000 = 0xaaaaaaaau;
    bump_allocate(0, 4); // exactly fills the 4-byte arena -> fatal trap
    expect("bump_trap_triggered", _DAT_00000000 == 0);
    DAT_00cf18e4 = arena;

    // ---- advance_cursor_copy (006e77cb) -----------------------------------
    alignas(16) std::uint8_t frame[0x20];
    std::uint32_t source_words[4] = {0x11, 0x22, 0x33, 0x44};
    *reinterpret_cast<std::uint8_t**>(frame + 0x1c) =
        reinterpret_cast<std::uint8_t*>(source_words);
    std::uint32_t cursor_dest[4] = {};
    const auto copied = advance_cursor_copy(frame, 3, 0, cursor_dest);
    expect("cursor_copy_count", copied == 3);
    expect("cursor_copy_data", cursor_dest[0] == 0x11 && cursor_dest[1] == 0x22 &&
                               cursor_dest[2] == 0x33 && cursor_dest[3] == 0);
    expect("cursor_advanced",
           *reinterpret_cast<std::uint8_t**>(frame + 0x1c) ==
               reinterpret_cast<std::uint8_t*>(source_words) + 12);

    // ---- copy_fixed_records (01003620) -------------------------------------
    for (int i = 0; i < 16; ++i) {
        _Src_00c754d0[i] = static_cast<std::byte>(i + 1);
    }
    DAT_00cf19a0 = 2;
    std::uint8_t fixed_dest[16] = {};
    const auto record_count = copy_fixed_records(fixed_dest);
    expect("fixed_records_count", record_count == 2);
    expect("fixed_records_data",
           std::memcmp(fixed_dest, _Src_00c754d0, 16) == 0);

    // ---- append_to_cursor (01003ca0) ---------------------------------------
    alignas(16) std::uint8_t cursor_buffer[32] = {};
    std::uint32_t cursor_variable = address(cursor_buffer);
    DAT_00cf1a08 = &cursor_variable;
    const std::uint16_t payload[2] = {0x1234, 0x5678};
    append_to_cursor(payload, 2);
    expect("append_copied", std::memcmp(cursor_buffer, payload, 4) == 0);
    expect("append_advanced", cursor_variable == address(cursor_buffer) + 4);

    std::uint32_t null_cursor = 0;
    DAT_00cf1a08 = &null_cursor;
    append_to_cursor(payload, 2);
    expect("append_noop_when_null", null_cursor == 0);

    // ---- init_payload_record (01002890) ------------------------------------
    alignas(16) std::uint8_t payload_arena_storage[0x100];
    std::int32_t payload_arena[3] = {
        static_cast<std::int32_t>(address(payload_arena_storage)), 0x100,
        static_cast<std::int32_t>(address(payload_arena_storage))};
    DAT_00cf18e4 = payload_arena;

    alignas(16) std::uint8_t source_blob[0x30];
    std::memset(source_blob, 0, sizeof source_blob);
    for (int i = 0; i < 12; ++i) {
        *(reinterpret_cast<std::uint32_t*>(source_blob) + i) =
            0x1000u + static_cast<std::uint32_t>(i);
    }
    std::uint8_t src_a[16];
    for (int i = 0; i < 16; ++i) src_a[i] = static_cast<std::uint8_t>(i + 1);
    source_blob[0x24] = 3;
    *reinterpret_cast<std::uint16_t*>(source_blob + 0x26) = 5;
    *reinterpret_cast<std::uint32_t*>(source_blob + 0x28) = address(src_a);

    std::uint8_t src_b[8];
    for (int i = 0; i < 8; ++i) src_b[i] = static_cast<std::uint8_t>(0x50 + i);
    *reinterpret_cast<std::uint16_t*>(source_blob + 0x22) = 4;
    *reinterpret_cast<std::uint32_t*>(source_blob + 0x2c) = address(src_b);

    DAT_00866368 = static_cast<std::int32_t>(0xabcdef01u);

    const auto rv = init_payload_record(
        nullptr, reinterpret_cast<const std::uint32_t*>(source_blob), 1, 1);
    expect("payload_return", rv == 1u);
    auto* record = DAT_00cf1920;
    bool header_ok = true;
    for (int i = 0; i < 12; ++i) {
        if (i == 4 || i == 5) continue;
        if (record[i] != *(reinterpret_cast<std::uint32_t*>(source_blob) + i)) {
            header_ok = false;
        }
    }
    expect("payload_header_copied", header_ok);
    expect("payload_table_a",
           record[4] == static_cast<std::uint32_t>(DAT_00866368));
    expect("payload_table_b",
           record[5] == static_cast<std::uint32_t>(DAT_00866368));
    expect("payload_size_a", DAT_00cf1918 == 3u * 5u);
    expect("payload_size_b", DAT_00cf191c == 4u * 2u);
    expect("payload_copy_a", std::memcmp(_Dst_00bb0a70, src_a, 15) == 0);
    expect("payload_copy_b", std::memcmp(_Dst_0096b490, src_b, 8) == 0);

    DAT_00cf1918 = 0xdeadbeefu;
    DAT_00cf191c = 0xdeadbeefu;
    const auto rv2 = init_payload_record(
        nullptr, reinterpret_cast<const std::uint32_t*>(source_blob), 0, 0);
    expect("payload_skip_both", rv2 == 1u && DAT_00cf1918 == 0xdeadbeefu &&
                               DAT_00cf191c == 0xdeadbeefu);

    if (failures != 0) {
        std::printf("scratch_buffers_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("scratch_buffers_smoke: all checks passed\n");
    return 0;
}
