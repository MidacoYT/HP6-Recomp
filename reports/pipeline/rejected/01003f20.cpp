#include <cstdint>

extern std::uint16_t DAT_009978e8[];
extern std::int32_t DAT_00bb0994;
extern std::int32_t DAT_00c754cc;
extern std::int32_t DAT_00cf1a34;

[[noreturn]] void __cdecl FUN_0063ccf0(char* format, ...);

void __cdecl FUN_01003f20(std::uint16_t param_1)
{
    if (static_cast<std::uint32_t>(DAT_00c754cc) <
        static_cast<std::uint32_t>(param_1)) {
        FUN_0063ccf0(const_cast<char*>(
            "ShaderManager::FreeShaderID - attempt to free a shader above the high water mark!\n"));
    }

    if (static_cast<std::uint32_t>(param_1) + 1 ==
        static_cast<std::uint32_t>(DAT_00c754cc)) {
        --DAT_00c754cc;
    }

    --DAT_00bb0994;
    DAT_009978e8[DAT_00bb0994] = param_1;
    --DAT_00cf1a34;
}
