#pragma once
#include <cstdint>

namespace hp6::projection {

// Set or clear one observed bit in the first word of a record. The original
// structure and field names are not established by the exports.
void set_flag_10(void* record, std::int32_t value); // 0x006d6bce
void set_flag_9(void* record, std::int32_t value);  // 0x006d71db
void set_flag_11(void* record, std::int32_t value); // 0x006d71fd
void initialize_record_header(void* destination, const void* source); // 0x006d7745
void set_flag_16(void* record, std::int32_t value); // 0x006d7220
void set_flag_17(void* record, std::int32_t value); // 0x006d7249
void set_flag_18(void* record, std::int32_t value); // 0x006d7272
void set_flag_19(void* record, std::int32_t value); // 0x006d729b
void set_flag_20(void* record, std::int32_t value); // 0x006d72c4
void set_flag_21(void* record, std::int32_t value); // 0x006d72ed
void set_flag_23(void* record, std::int32_t value); // 0x006d7316
void set_flag_24(void* record, std::int32_t value); // 0x006d733f
void set_flag_22(void* record, std::int32_t value); // 0x006d7368

}
