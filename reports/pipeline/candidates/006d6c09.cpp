std::uint8_t __fastcall FUN_006d6c09(std::uint32_t* param_1)
{
    const std::uint32_t value = *param_1 & 0xff;

    if (value == 0x69) return 0x80;
    if (value == 0x6f) return 0x20;
    if (value == 0x7c) return 0x10;
    if (value == 0x81) return 1;

    if (FUN_006d6b7f(value) != 0) return 4;

    const std::uint32_t result = FUN_006d6ba4(value);
    return static_cast<std::uint8_t>(
        -static_cast<int>(static_cast<std::uint8_t>(result) != 0) & 2);
}
