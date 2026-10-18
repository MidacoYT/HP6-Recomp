#include <cstdint>
#include <cstring>

std::int32_t __cdecl FUN_006e77cb(
    std::int32_t param_1,
    std::int32_t param_2,
    std::uint32_t param_3,
    void* param_4)
{
    // The 32-bit pointer stored at offset 0x1c has no evidenced member name.
    auto* cursor = reinterpret_cast<std::uint32_t*>(
        static_cast<std::uintptr_t>(static_cast<std::uint32_t>(param_1)) + 0x1c);

    const std::uint32_t byte_count =
        static_cast<std::uint32_t>(param_2) << 2;

    std::memcpy(
        param_4,
        reinterpret_cast<const void*>(static_cast<std::uintptr_t>(*cursor)),
        byte_count);

    *cursor += byte_count;
    return param_2;
}
