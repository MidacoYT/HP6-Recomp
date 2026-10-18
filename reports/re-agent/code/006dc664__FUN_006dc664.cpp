unsigned int __cdecl FUN_006dc664(unsigned char* param_1)
{
    unsigned int result = 0;
    const int value = static_cast<int>(FUN_006dc5f9(param_1));

    if (value == 1) {
        result = ((static_cast<unsigned int>(param_1[2]) << 8)
                | static_cast<unsigned int>(param_1[3])) + 4;
    } else if (1 < value && value < 4) {
        result = FUN_006dc532(param_1 + 0x0c, 4);
    }

    return result;
}