#pragma once
#include <cstdint>

namespace hp6::projection {

void release_record_chain(void* first); // 0x006d81e5
std::uint8_t remove_record_by_kind(void* first,
                                   std::uint32_t kind); // 0x006d700a, low byte
std::uint8_t insert_record_after_kind(void* first,
                                      void* incoming); // 0x006d707c, low byte

}
