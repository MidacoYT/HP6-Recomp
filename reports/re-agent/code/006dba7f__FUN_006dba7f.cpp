#include <cstddef>
#include <cstdint>

static_assert(sizeof(void*) == 4);

struct UnknownRecord {
    std::byte unknown_00[0x28];
    std::uint32_t field_28;  // Address of 12-byte entries; member name unresolved
    std::int32_t field_2c;   // Member name unresolved
    std::byte unknown_30[4];
    std::int32_t field_34;   // Member name unresolved
    std::byte unknown_38[12];
    std::int32_t field_44;   // Member name unresolved
};

extern std::int32_t __fastcall FUN_006db3f9(
    std::uint32_t*, std::uint32_t*, std::uint32_t*);

void __cdecl FUN_006dba7f(
    std::uint32_t* param_1,
    std::int32_t param_2,
    std::int32_t param_3,
    std::int32_t param_4,
    std::int32_t param_5)
{
    std::uint32_t local_c;
    std::uint32_t local_8;

    const std::int32_t result = FUN_006db3f9(param_1, &local_c, &local_8);
    if (result != 0) return;

    auto* record = reinterpret_cast<UnknownRecord*>(local_8);
    if (param_2 <= 0 || param_2 > record->field_2c) return;
    if (param_2 == record->field_2c &&
        (param_3 != 0 || param_4 != 0)) return;
    if (param_5 <= 0 && param_5 != -1 && param_5 != -2) return;
    if (param_5 > record->field_34 || record->field_44 != 0) return;

    const std::uint32_t entry_address =
        record->field_28 - 0xc +
        static_cast<std::uint32_t>(param_2) * 0xc;
    auto* entry = reinterpret_cast<std::int32_t*>(entry_address);
    entry[0] = param_3;
    entry[1] = param_4;
    entry[2] = param_5;
}