#pragma once
#include <cstdint>

namespace hp6::projection {

// Operation values 1, 2 and 3 are observed in the x86 branches. Their
// higher-level names remain provisional until callers are reconstructed.
std::uint32_t measure_collection_update(const void* first,
                                        const void* input,
                                        std::int32_t operation); // 0x006d896e
std::uint32_t measure_record_replacement(const void* first,
                                         const void* input,
                                         std::int32_t operation); // 0x006d6ec3

}
