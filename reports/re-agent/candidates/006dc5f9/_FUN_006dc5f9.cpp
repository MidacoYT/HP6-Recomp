unsigned int __cdecl FUN_006dc5f9(unsigned char* param_1)
{
    const unsigned int first = param_1[0];
    const unsigned int second = param_1[1];
    if (((first << 8) | second) == 0xc0fb) {
        return 1;
    }

    unsigned int value = FUN_006dc532(param_1, 4);
    if (value == 0x42494746) {
        return 2;
    }

    value = FUN_006dc532(param_1, 4);
    if ((value & 0xffffff00) == 0x42494700) {
        return 3;
    }

    value = FUN_006dc532(param_1, 4);
    if ((value & 0xffffff00) == 0x53687000) {
        return 2;
    }

    return 0;
}
