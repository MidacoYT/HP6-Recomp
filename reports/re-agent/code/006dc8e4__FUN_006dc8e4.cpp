unsigned int __cdecl FUN_006dc8e4(unsigned char* param_1)
{
    unsigned int result = 0;
    const int type = FUN_006dc5f9(param_1);

    if (type == 1) {
        result = (static_cast<unsigned int>(param_1[4]) << 8) |
                 static_cast<unsigned int>(param_1[5]);
    } else if (type > 1 && type < 4) {
        result = FUN_006dc532(param_1 + 8, 4);
    }

    return result;
}