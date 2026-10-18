#include <cstdint>
#include <cstring>

struct Object_006b6833 {
    char field_00[0x40];
    std::uint32_t field_40;
    std::uint32_t field_44;
    std::uint32_t field_48;
    std::uint32_t field_4c;
    std::uint32_t field_50;
    std::uint32_t field_54;
    std::uint32_t field_58;
    std::uint32_t field_5c;
    std::uint8_t field_60;

    void __thiscall FUN_006b6833(
        const char* param_1,
        std::uint32_t param_2,
        std::uint32_t param_3,
        std::uint32_t param_4,
        std::uint32_t param_5,
        const std::uint32_t* param_6);
};

void __fastcall FUN_006b6808(Object_006b6833* param_1);

void __thiscall Object_006b6833::FUN_006b6833(
    const char* param_1,
    std::uint32_t param_2,
    std::uint32_t param_3,
    std::uint32_t param_4,
    std::uint32_t param_5,
    const std::uint32_t* param_6)
{
    FUN_006b6808(this);
    std::strncpy(field_00, param_1, 0x40);

    field_40 = param_2;
    field_44 = param_3;
    field_48 = param_4;
    field_4c = param_5;

    if (param_6 != nullptr) {
        field_50 = param_6[0];
        field_54 = param_6[1];
        field_58 = param_6[2];
        field_5c = param_6[3];
    }

    field_60 = 0;
}