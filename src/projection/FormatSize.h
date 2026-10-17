#pragma once
#include <cstdint>

namespace hp6::projection {

std::uint8_t is_block_format_code(std::uint32_t code); // 0x006d7187
std::uint32_t map_format_unit(std::int32_t value); // 0x006d74af
void format_geometry(std::uint32_t code, std::uint32_t output[3]); // 0x006d8c7d
std::int32_t extra_block_count(std::uint32_t code, std::int32_t rows,
                               std::int32_t columns); // 0x006d8d62
std::int32_t estimate_record_size(const void* source); // 0x006d74ce
std::int32_t estimate_variant_04_size(const void* source); // 0x006d760a
std::int32_t estimate_variant_10_size(const void* source); // 0x006d768d
std::uint32_t estimate_collection_size(const void* source); // 0x006d76a6
std::uint32_t record_storage_size(const void* source); // 0x006d8864
std::uint32_t fixed_clip_storage_size(); // 0x006d7667
std::uint32_t comment_storage_size(const void* source); // 0x006d766b
std::uint32_t eagl_storage_size(const void* source); // 0x006d767c

}
