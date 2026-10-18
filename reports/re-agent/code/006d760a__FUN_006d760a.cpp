#include <cstddef>
#include <cstdint>

struct FUN_006d760a_record {
    std::byte unknown_00[0x14];
    std::uint32_t field_14;
    std::byte unknown_18[0x3c];
    std::uint32_t field_54;
    std::byte unknown_58[0x04];
    std::int32_t field_5c;
    std::int32_t field_60;
};

static_assert(offsetof(FUN_006d760a_record, field_14) == 0x14);
static_assert(offsetof(FUN_006d760a_record, field_54) == 0x54);
static_assert(offsetof(FUN_006d760a_record, field_5c) == 0x5c);
static_assert(offsetof(FUN_006d760a_record, field_60) == 0x60);

extern std::uint8_t DAT_0071b150[];
extern std::int32_t __cdecl FUN_006d74af(std::uint32_t);

int __cdecl FUN_006d760a(int param_1)
{
    // The supplied evidence does not identify the record or its members.
    const auto* record =
        reinterpret_cast<const FUN_006d760a_record*>(param_1);

    const std::uint32_t type = record->field_14;
    const std::uint8_t scale = DAT_0071b150[record->field_54 & 0xff];
    std::int32_t value = record->field_5c;
    std::int32_t result = 0;

    if (((value == 0) && (type == 0x79)) || (type == 0x7b)) {
        value = FUN_006d74af(DAT_0071b150[type & 0xff]);
    }

    if (record->field_60 == 0) {
        result = static_cast<std::int32_t>(
            static_cast<std::uint32_t>(value) *
            static_cast<std::uint32_t>(scale)) / 8;
    }

    return result + 0x20;
}