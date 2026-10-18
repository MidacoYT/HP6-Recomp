#include <cstdint>

// The evidence identifies these offsets, but does not establish their member names.
struct Unknown_006b2c65 {
    std::int32_t field_00;
    std::int32_t field_04;
    std::int32_t field_08;

    void __thiscall FUN_006b2c65(std::int32_t param_1);
};

void __thiscall Unknown_006b2c65::FUN_006b2c65(std::int32_t param_1)
{
    auto* node = reinterpret_cast<Unknown_006b2c65*>(param_1);

    const std::int32_t value = field_00 == 0 ? 0 : field_00 + 8;
    node->field_08 = value;
    field_08 = field_08 + 1;
    field_00 = param_1;

    if (node->field_08 == 0) {
        field_04 = param_1;
    }
}
