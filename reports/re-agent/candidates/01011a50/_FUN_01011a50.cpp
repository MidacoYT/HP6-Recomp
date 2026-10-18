void __stdcall FUN_01011a50()
{
    std::uint32_t value;
    std::uint8_t shift;

    if (DAT_008663d5 == 0) {
        value = reinterpret_cast<const std::uint32_t*>(0x008928c0)
                    [DAT_008663b8]
                << (1U - (static_cast<std::uint32_t>(DAT_00cf1964) < 0x180U));
        shift = static_cast<std::uint8_t>(
            1U - (static_cast<std::uint32_t>(DAT_00cf1964) < 0x200U));
    } else {
        const auto address =
            static_cast<std::uint32_t>(DAT_00cf19d0) +
            static_cast<std::uint32_t>(DAT_008663d8) * 0x584U + 0x474U;
        value = (static_cast<std::uint32_t>(
                     -static_cast<std::uint32_t>(
                         (*reinterpret_cast<const std::uint32_t*>(address) & 0x800U) != 0))
                 & 0x20U) + 0x20U;

        if (static_cast<std::uint32_t>(DAT_00cf19bc) >= 2U) {
            value = 0x80U;
        }
        shift = static_cast<std::uint8_t>(DAT_008663b8);
    }

    DAT_008663dc =
        (value << (shift & 0x1fU)) >>
        (1U - (static_cast<std::uint32_t>(DAT_00cf1964) > 0x40U));

    if (DAT_00cf1974 != 0) {
        DAT_008663dc = DAT_00cf1974;
    }
}
