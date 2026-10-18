#include <cstdint>
#include <cstring>

void __fastcall FUN_006b687f(std::uint32_t* param_1);

// The supplied evidence does not identify member names for offsets 0x40–0x4c.
void __thiscall FUN_006b6894(
    void* self, char* param_1, std::uint32_t* param_2)
{
    FUN_006b687f(static_cast<std::uint32_t*>(self));
    std::strncpy(static_cast<char*>(self), param_1, 0x40);

    if (param_2 != nullptr) {
        auto* fields = reinterpret_cast<std::uint32_t*>(
            static_cast<char*>(self) + 0x40);
        fields[0] = param_2[0];
        fields[1] = param_2[1];
        fields[2] = param_2[2];
        fields[3] = param_2[3];
    }
}