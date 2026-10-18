unsigned int __cdecl FUN_006b343e(int* param_1)
{
    if (*param_1 < 0x50) {
        FUN_006b3b0e(&DAT_00851380, 0x50);
    }

    FUN_006b3b04();

    const int value = param_1[0xb];
    _DAT_008513d4 = value;
    return (static_cast<unsigned int>(value) & 0xffffff00u) | 1u;
}
