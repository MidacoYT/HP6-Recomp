#pragma once
#include <cstdint>

namespace hp6::projection {

// Records are linked by a signed relative offset at byte +4.
std::uint32_t measure_clone_chain(const void* first); // 0x006d88d2
void clone_record_chain_into(const void* first, void* storage); // 0x006d86d5
void* clone_record_chain_allocated(const void* first,
                                   std::uint32_t context); // 0x006d8816

}
