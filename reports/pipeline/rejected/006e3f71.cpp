#include <cstdint>

extern std::uint32_t __cdecl FUN_006e0778(std::int32_t param_1);

std::uint32_t* __cdecl FUN_006e3f71(std::uint32_t param_1)
{
    if (static_cast<std::int32_t>(param_1) < 0) {
        return nullptr;
    }

    const std::uint32_t base = FUN_006e0778(param_1 & 0xff);
    if (base == 0) {
        return nullptr;
    }

    auto* entry = reinterpret_cast<std::uint32_t*>(
        static_cast<std::uintptr_t>(
            *reinterpret_cast<std::uint32_t*>(
                static_cast<std::uintptr_t>(base) + 0x120)));

    while (entry != nullptr && entry[3] != param_1) {
        entry = reinterpret_cast<std::uint32_t*>(
            static_cast<std::uintptr_t>(entry[0]));
    }

    return entry;
}
