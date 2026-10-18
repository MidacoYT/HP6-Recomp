void __cdecl FUN_005d9210(char param_1)
{
    if (param_1 != '\0') {
        SystemParametersInfoA(0x3b, 8, &pvParam_00866f14, 0);
        SystemParametersInfoA(0x35, 8, &pvParam_00866f1c, 0);
        SystemParametersInfoA(0x33, 0x18, &pvParam_00866f24, 0);
        return;
    }

    struct {
        PVOID local_30;
        uint32_t local_2c;
    } first;
    first.local_30 = pvParam_00866f14;
    first.local_2c = DAT_00866f18;
    if ((DAT_00866f18 & 1) == 0) {
        first.local_2c = DAT_00866f18 & 0xfffffff3;
        SystemParametersInfoA(0x3b, 8, &first.local_30, 0);
    }

    struct {
        PVOID local_28;
        uint32_t local_24;
    } second;
    second.local_28 = pvParam_00866f1c;
    second.local_24 = DAT_00866f20;
    if ((DAT_00866f20 & 1) == 0) {
        second.local_24 = DAT_00866f20 & 0xfffffff3;
        SystemParametersInfoA(0x35, 8, &second.local_28, 0);
    }

    struct {
        PVOID local_20;
        uint32_t local_1c;
        uint32_t local_18;
        uint32_t local_14;
        uint32_t local_10;
        uint32_t local_c;
    } third;
    third.local_20 = pvParam_00866f24;
    third.local_18 = DAT_00866f2c;
    third.local_14 = DAT_00866f30;
    third.local_10 = DAT_00866f34;
    third.local_c = DAT_00866f38;
    if ((DAT_00866f28 & 1) == 0) {
        third.local_1c = DAT_00866f28 & 0xfffffff3;
        SystemParametersInfoA(0x33, 0x18, &third.local_20, 0);
    }
}
