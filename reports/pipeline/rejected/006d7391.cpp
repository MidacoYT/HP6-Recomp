#include <cstdint>

struct FUN_006d7391_object {
    std::uint8_t unknown_00[0x10];
    std::uint32_t field_10;
    std::uint32_t field_14;
    std::uint32_t field_18;
    std::uint32_t field_1c;

    void __thiscall FUN_006d7391(
        std::uint32_t* param_1,
        std::uint32_t* param_2,
        std::uint32_t* param_3,
        std::uint32_t* param_4);
};

void __thiscall FUN_006d7391_object::FUN_006d7391(
    std::uint32_t* param_1,
    std::uint32_t* param_2,
    std::uint32_t* param_3,
    std::uint32_t* param_4)
{
    *param_1 = field_10;
    *param_2 = field_14;
    *param_3 = field_18;
    *param_4 = field_1c;
}
