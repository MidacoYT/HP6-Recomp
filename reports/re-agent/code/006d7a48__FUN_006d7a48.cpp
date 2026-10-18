#include <cstdint>

struct FUN_006d7a48_Output {
    std::uint8_t field_00[8];
    std::uint32_t field_08;
    std::uint32_t field_0c;
    std::uint32_t field_10;
};

// The owner type and the three output field names are unresolved.
void __thiscall FUN_006d7a48(
    void* this_,
    FUN_006d7a48_Output* param_1)
{
    param_1->field_08 =
        (*static_cast<std::uint32_t*>(this_) >> 10) & 1u;
    param_1->field_0c =
        (*static_cast<std::uint32_t*>(this_) >> 11) & 1u;
    param_1->field_10 =
        (*static_cast<std::uint32_t*>(this_) >> 9) & 1u;
}