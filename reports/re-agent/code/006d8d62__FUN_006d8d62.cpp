#include <cstdint>

void __cdecl FUN_006d8c7d(std::uint32_t param_1, std::int32_t* param_2);

std::int32_t __cdecl FUN_006d8d62(
    std::uint32_t param_1,
    std::int32_t param_2,
    std::int32_t param_3)
{
    std::int32_t values[3];
    FUN_006d8c7d(param_1, values);
    return (param_2 * param_3) / (values[1] * values[0]);
}