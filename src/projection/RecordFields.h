#pragma once
#include <cstdint>

namespace hp6::projection {

// Source and destination are byte records with the observed offsets. Their
// owning types and field names have not been recovered.
std::uint8_t is_projection_code(std::int32_t code); // 0x006d6b7f
std::uint8_t is_table_projection_code(std::uint32_t code); // 0x006d6ba4
std::uint32_t classify_record(const void* source); // 0x006d6c09
std::uint8_t is_block_code(std::int32_t code); // 0x006d7170
std::uint8_t is_selected_format_code(std::uint32_t code); // 0x006d81a4, low byte
void copy_four_fields(const void* source, void* first, void* second,
                      void* third, void* fourth); // 0x006d7391
void initialize_record(void* destination);         // 0x006d73b8; 0xc0 bytes
void* initialize_record_and_return(void* destination); // 0x006d744b
void write_projection_list(const void* first, void* destination); // 0x006d7c07
void write_common_flags(const void* source, void* destination); // 0x006d7a48
void write_variant_02(const void* source, void* destination);   // 0x006d7a73
void write_variant_04(const void* source, void* destination);   // 0x006d7b14
void write_variant_100(const void* source, void* destination);  // 0x006d7b49
void write_variant_10(const void* source, void* destination);   // 0x006d7b7a
void write_variant_80(const void* source, void* destination);   // 0x006d7baf
void write_variant_20(const void* source, void* destination);   // 0x006d7bd2

}
