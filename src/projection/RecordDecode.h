#pragma once
#include <cstdint>

namespace hp6::projection {

void set_relative_payload(void* record, const void* payload,
                          std::uint32_t size); // 0x00688550
void link_next_record(void* record, const void* next); // 0x006d6bf1
void decode_variant_02(const void* source, void* destination); // 0x006d7cd2
void decode_variant_100(const void* source, void* destination); // 0x006d7827
void decode_variant_20(const void* source, void* destination); // 0x006d79ce
void decode_variant_04(const void* source, void* destination); // 0x006d778a
void decode_variant_10(const void* source, void* destination); // 0x006d78a8
void clone_variant_02(const void* source, void* destination); // 0x006d822a
void clone_variant_04(const void* source, void* destination); // 0x006d82ec
void clone_variant_100(const void* source, void* destination); // 0x006d83ab
void clone_variant_10(const void* source, void* destination); // 0x006d8466
void clone_variant_80(const void* source, void* destination); // 0x006d852e
void clone_variant_20(const void* source, void* destination); // 0x006d8619
void decode_variant_80(const void* source, void* destination); // 0x006d793d
void assemble_projection_records(void* storage, const void* source); // 0x006d7dea


}
