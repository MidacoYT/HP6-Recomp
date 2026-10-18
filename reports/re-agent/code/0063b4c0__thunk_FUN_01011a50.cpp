#include <cstdint>

extern std::uint32_t DAT_008663b8;
extern std::uint8_t  DAT_008663d5;
extern std::uint32_t DAT_008663d8;
extern std::uint32_t DAT_008663dc;
extern std::uint32_t DAT_00cf1964;
extern std::uint32_t DAT_00cf1974;
extern std::uint32_t DAT_00cf19bc;
extern std::uint32_t DAT_00cf19d0;

void __stdcall thunk_FUN_01011a50()
{
    std::uint32_t value;
    std::uint8_t shift;

    if (DAT_008663d5 == 0) {
        value = *reinterpret_cast<const std::uint32_t*>(
                    0x008928c0u + DAT_008663b8 * 4u)
                << ((1u - (DAT_00cf1964 < 0x180u)) & 0x1fu);
        shift = static_cast<std::uint8_t>(
            1u - (DAT_00cf1964 < 0x200u));
    } else {
        value = (-static_cast<std::uint32_t>(
                     (*reinterpret_cast<const std::uint32_t*>(
                         DAT_008663d8 * 0x584u + 0x474u + DAT_00cf19d0)
                      & 0x800u) != 0u)
                 & 0x20u) + 0x20u;
        if (DAT_00cf19bc > 1u) {
            value = 0x80u;
        }
        shift = static_cast<std::uint8_t>(DAT_008663b8);
    }

    DAT_008663dc = (value << (shift & 0x1fu))
                  >> ((1u - (0x40u < DAT_00cf1964)) & 0x1fu);

    if (DAT_00cf1974 != 0u) {
        DAT_008663dc = DAT_00cf1974;
    }
}