#include <cstdint>

extern std::uint8_t DAT_0089b438;
extern std::uint32_t __cdecl FUN_006e0778(std::int32_t param_1);
extern void __cdecl FUN_006f4eb9(std::uint32_t* param_1, std::int32_t param_2);

std::int32_t __cdecl FUN_006e71e1(std::int32_t param_1, std::int32_t param_2)
{
    if (DAT_0089b438 == 0) {
        return -10;
    }

    const std::uint32_t entryAddress = FUN_006e0778(param_1);
    if (entryAddress == 0) {
        return -8;
    }

    const auto* entry = reinterpret_cast<const std::uint32_t*>(
        static_cast<std::uintptr_t>(entryAddress));
    auto* value = reinterpret_cast<std::uint32_t*>(
        static_cast<std::uintptr_t>(*entry));
    FUN_006f4eb9(value, param_2);
    return 0;
}
