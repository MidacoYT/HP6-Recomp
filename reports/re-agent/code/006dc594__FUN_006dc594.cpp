std::uint64_t __cdecl FUN_006dc594(int param_1, int param_2)
{
    std::uint32_t upper = 0;
    std::uint32_t lower = 0;
    int index = 0;

    if (0 < param_2) {
        do {
            upper = (upper << 8) | (lower >> 24);
            lower = static_cast<std::uint32_t>(
                        *reinterpret_cast<const unsigned char*>(param_1 + index))
                  | (lower << 8);
            index = index + 1;
        } while (index < param_2);
    }

    return (static_cast<std::uint64_t>(upper) << 32) | lower;
}