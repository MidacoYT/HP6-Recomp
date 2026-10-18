#include <cstddef>
#include <cstdint>
#include <cstdio>

// Member identities are unresolved; these names preserve the known offsets.
struct FUN_006d793d_Object {
    std::uint8_t field_0x00;
    std::uint8_t unknown_0x01[7];
    std::int32_t field_0x08;  // Relative string offset; zero means null.
    std::uint8_t unknown_0x0c[4];
    char field_0x10[1];
};

struct FUN_006d793d_Input {
    std::uint8_t unknown_0x00[0x84];
    std::int32_t field_0x84;
};

static_assert(offsetof(FUN_006d793d_Object, field_0x08) == 0x08);
static_assert(offsetof(FUN_006d793d_Object, field_0x10) == 0x10);
static_assert(offsetof(FUN_006d793d_Input, field_0x84) == 0x84);

void __thiscall FUN_006d7745(void* self, std::int32_t input);
void __thiscall FUN_00688550(void* self, std::int32_t address, std::int32_t value);

void __thiscall FUN_006d793d(void* self, std::int32_t param_1)
{
    auto* object = static_cast<FUN_006d793d_Object*>(self);
    auto* input = reinterpret_cast<FUN_006d793d_Input*>(param_1);

    FUN_006d7745(self, param_1);
    const std::int32_t value = input->field_0x84;
    object->field_0x00 = 0x69;
    FUN_00688550(self, reinterpret_cast<std::int32_t>(object->field_0x10), value);

    char* destination = reinterpret_cast<char*>(object->field_0x08);
    if (destination != nullptr) {
        destination = reinterpret_cast<char*>(object) + object->field_0x08;
    }
    std::sprintf(destination, "EAGL%d", value);
}