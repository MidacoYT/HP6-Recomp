#include <cstdint>

extern std::int32_t* DAT_00cf18e4;
extern volatile std::uint32_t _DAT_00000000;

void __fastcall FUN_0063b850(std::uint32_t param_1, std::int32_t param_2)
{
    std::int32_t* piVar1 = DAT_00cf18e4 + 1;
    std::int32_t iVar2 = *DAT_00cf18e4;

    std::uint32_t uVar3 =
        ((static_cast<std::uint32_t>(param_2) + 3U) & 0xfffffffcU) +
        static_cast<std::uint32_t>(DAT_00cf18e4[2]);

    DAT_00cf18e4[2] = static_cast<std::int32_t>(uVar3);

    if ((static_cast<std::uint32_t>(*piVar1) +
         static_cast<std::uint32_t>(iVar2)) <= uVar3) {
        _DAT_00000000 = 0;
    }
}