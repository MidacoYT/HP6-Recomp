#include "projection/RecordFlags.h"
#include <cstdint>
#include <cstring>

int main() {
    using Setter = void (*)(void*, std::int32_t);
    const Setter setters[] = {
        hp6::projection::set_flag_16, hp6::projection::set_flag_17,
        hp6::projection::set_flag_18, hp6::projection::set_flag_19,
        hp6::projection::set_flag_20, hp6::projection::set_flag_21,
        hp6::projection::set_flag_22, hp6::projection::set_flag_23,
        hp6::projection::set_flag_24,
    };
    for (std::uint32_t bit = 16; bit <= 24; ++bit) {
        const std::uint32_t mask = 1u << bit;
        std::uint32_t word = 0x00001234u;
        setters[bit - 16](&word, 1);
        if (word != (0x00001234u | mask)) return 1;
        word = 0xffffffffu;
        setters[bit - 16](&word, 0);
        if (word != ~mask) return 2;
        setters[bit - 16](&word, -1);
        if (word != ~mask) return 3;
    }
    std::uint32_t source[5] = {};
    source[2] = 1;
    source[3] = 0;
    source[4] = 1;
    std::uint32_t destination[4] = {0xffffffffu, 7u, 8u, 9u};
    hp6::projection::initialize_record_header(destination, source);
    const std::uint32_t expected = (0xffffffffu & ~(1u << 11)) | 0x100u;
    if (destination[0] != expected || destination[1] != 0 ||
        destination[2] != 0 || destination[3] != 0) return 4;
    hp6::projection::set_flag_9(destination, 0);
    hp6::projection::set_flag_10(destination, -1);
    hp6::projection::set_flag_11(destination, 1);
    if ((destination[0] & (7u << 9)) != (1u << 11)) return 5;
    return 0;
}
