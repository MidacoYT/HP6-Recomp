#include <cstddef>
#include <cstdint>

// The member at offset 0x0c has no evidence-backed name.
struct FUN_006d8685_Context {
    std::byte unknown_00[0x0c];
    std::int32_t field_0c;
};

std::uint32_t* __thiscall FUN_006d8685(
    FUN_006d8685_Context* self,
    std::uint32_t param_1)
{
    auto* result = reinterpret_cast<std::uint32_t*>(
        FUN_006d6ab6("cl_cmnt", self->field_0c + 0x10,
                     0x10, 0x10, param_1, "shpclone.cpp", 0x136));

    FUN_006d8619(self, result);
    result[1] = 0;
    result[0] = (result[0] & 0xfffffbffu) | 0x400u;
    return result;
}
