#include <cstdint>

extern std::uint16_t DAT_009978e8[];
extern std::int32_t DAT_00bb0994;
extern std::int32_t DAT_00c754cc;
extern std::int32_t DAT_00cf1a34;

[[noreturn]] void __cdecl FUN_0063ccf0(char* format, ...);

std::uint32_t __stdcall FUN_01003ea0()
{
    if (DAT_00cf1a34 < 0x600) {
        const std::uint16_t shaderId = DAT_009978e8[DAT_00bb0994];
        ++DAT_00bb0994;

        const std::uint32_t nextId = static_cast<std::uint32_t>(shaderId) + 1;
        if (DAT_00c754cc < static_cast<std::int32_t>(nextId)) {
            DAT_00c754cc = static_cast<std::int32_t>(nextId);
        }

        ++DAT_00cf1a34;
        return (nextId & 0xffff0000u) | shaderId;
    }

    FUN_0063ccf0(const_cast<char*>(
        "ShaderManager::AllocateShaderID - MAXIMUM number of shaders exceeded!\n"));
}
