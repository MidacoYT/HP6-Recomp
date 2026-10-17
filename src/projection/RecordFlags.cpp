#include "projection/RecordFlags.h"
#include <cstddef>
#include <cstring>

namespace hp6::projection {

static void set_flag(void* record, std::uint32_t mask, std::int32_t value) {
    std::uint32_t word;
    std::memcpy(&word, record, sizeof word);
    word = (word & ~mask) | (value == 1 ? mask : 0u);
    std::memcpy(record, &word, sizeof word);
}

static std::uint32_t read_word(const void* record, std::size_t offset) {
    std::uint32_t word;
    std::memcpy(&word, static_cast<const std::byte*>(record) + offset, sizeof word);
    return word;
}

static void write_word(void* record, std::size_t offset, std::uint32_t word) {
    std::memcpy(static_cast<std::byte*>(record) + offset, &word, sizeof word);
}

// ReAgent's initial candidates and the x86 DEC/NEG/SBB sequence agree that
// exactly value 1 sets the selected bit; all other values clear it.
void set_flag_10(void* record, std::int32_t value) { set_flag(record, 1u << 10, value); }
void set_flag_9(void* record, std::int32_t value) { set_flag(record, 1u << 9, value); }
void set_flag_11(void* record, std::int32_t value) { set_flag(record, 1u << 11, value); }

void initialize_record_header(void* destination, const void* source) {
    write_word(destination, 0, read_word(destination, 0) | 0x100u);
    set_flag_10(destination, static_cast<std::int32_t>(read_word(source, 0x08)));
    set_flag_11(destination, static_cast<std::int32_t>(read_word(source, 0x0c)));
    set_flag_9(destination, static_cast<std::int32_t>(read_word(source, 0x10)));
    write_word(destination, 0x08, 0);
    write_word(destination, 0x0c, 0);
    write_word(destination, 0x04, 0);
}

void set_flag_16(void* record, std::int32_t value) { set_flag(record, 1u << 16, value); }
void set_flag_17(void* record, std::int32_t value) { set_flag(record, 1u << 17, value); }
void set_flag_18(void* record, std::int32_t value) { set_flag(record, 1u << 18, value); }
void set_flag_19(void* record, std::int32_t value) { set_flag(record, 1u << 19, value); }
void set_flag_20(void* record, std::int32_t value) { set_flag(record, 1u << 20, value); }
void set_flag_21(void* record, std::int32_t value) { set_flag(record, 1u << 21, value); }
void set_flag_23(void* record, std::int32_t value) { set_flag(record, 1u << 23, value); }
void set_flag_24(void* record, std::int32_t value) { set_flag(record, 1u << 24, value); }
void set_flag_22(void* record, std::int32_t value) { set_flag(record, 1u << 22, value); }

}
