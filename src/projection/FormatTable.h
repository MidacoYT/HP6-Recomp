#pragma once
#include <cstdint>

namespace hp6::projection {

inline constexpr std::uint8_t format_table[256] = {
#include "projection/FormatTable.inc"
};

}
