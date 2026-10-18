#include <bit>
#include <cstdint>

extern const std::uint8_t DAT_0071b150[];
std::uint8_t __cdecl FUN_006d7170(std::uint32_t);
std::uint8_t __cdecl FUN_006d7187(std::uint32_t);
std::int32_t __cdecl FUN_006d8d62(std::uint32_t, std::uint32_t, std::uint32_t);

std::int32_t __cdecl FUN_006d74ce(std::int32_t param_1)
{
    // The supplied evidence does not identify the structure or its member names.
    const auto base = static_cast<std::uintptr_t>(static_cast<std::uint32_t>(param_1));
    const auto read = [base](std::uintptr_t offset) -> std::uint32_t {
        return *reinterpret_cast<const std::uint32_t*>(base + offset);
    };

    const auto level_limit = std::bit_cast<std::int32_t>(read(0x28));
    const auto format = read(0x14);
    std::uint32_t width = read(0x18);
    std::uint32_t height = read(0x1c);
    std::uint32_t total = 0;

    if (read(0x50) != 0)
        return 0x20;

    const auto table_value = DAT_0071b150[format & 0xff];
    const std::uint32_t bits = table_value == 0x0f ? 0x10 : table_value;

    if (std::bit_cast<std::int32_t>(width) < 1 ||
        std::bit_cast<std::int32_t>(height) < 1)
        return 0;

    if (level_limit >= 0) {
        const auto aligned_four = FUN_006d7170(format);
        const auto extra_size = FUN_006d7187(format);
        std::int32_t level = 0;

        do {
            std::uint32_t row_width;
            std::uint32_t row_count;

            if (aligned_four != 0) {
                row_width = (width + 3) & 0xfffffffc;
                row_count = (height + 3) & 0xfffffffc;
            } else {
                row_count = height;
                if (format == 0x68) {
                    row_width = (width + 1) & 0xfffffffe;
                } else if (extra_size != 0) {
                    row_width = (width + 0x1f) & 0xffffffe0;
                    row_count = (height + 0x0f) & 0xfffffff0;
                } else {
                    row_width = width;
                }
            }

            const auto row_bytes =
                std::bit_cast<std::int32_t>(row_width * bits + 7) >> 3;
            total += static_cast<std::uint32_t>(row_bytes) * row_count;

            if (extra_size != 0)
                total += static_cast<std::uint32_t>(
                    FUN_006d8d62(format, row_count, row_width));

            if (width == 1 && height == 1)
                break;

            width >>= 1;
            height >>= 1;
            if (width < 1) width = 1;
            if (height < 1) height = 1;
            ++level;
        } while (level <= level_limit);
    }

    return std::bit_cast<std::int32_t>(total + 0x20);
}