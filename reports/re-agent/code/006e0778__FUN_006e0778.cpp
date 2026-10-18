#include <cstdint>

extern std::uint8_t DAT_0089b2fe;
extern std::uint32_t DAT_0089b200[];

std::uint32_t __cdecl FUN_006e0778(std::int32_t param_1)
{
    if (param_1 < static_cast<std::int32_t>(DAT_0089b2fe) &&
        param_1 >= 0) {
        return DAT_0089b200[param_1];
    }
    return 0;
}