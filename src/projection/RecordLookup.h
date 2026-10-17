#pragma once
#include <cstdint>

namespace hp6::projection {

void* find_record_by_kind(void* first, std::uint32_t kind); // 0x006d6c5c
void* find_variant_02(void* first); // 0x006d70e4
void* find_variant_04(void* first); // 0x006d70ec
void* find_variant_10(void* first); // 0x006d70ff
void* find_variant_20(void* first); // 0x006d7112

}
