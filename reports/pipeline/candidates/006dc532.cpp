unsigned int __cdecl FUN_006dc532(unsigned char* param_1, int param_2)
{
    if (param_2 == 1) {
        return param_1[0];
    }

    if (param_2 == 2) {
        return (static_cast<unsigned int>(param_1[0]) << 8) | param_1[1];
    }

    unsigned int upper;
    unsigned char last;

    if (param_2 == 3) {
        upper = (static_cast<unsigned int>(param_1[0]) << 8) | param_1[1];
        last = param_1[2];
    } else {
        if (param_2 != 4) {
            return 0;
        }
        last = param_1[3];
        upper = ((static_cast<unsigned int>(param_1[0]) << 8) | param_1[1]) << 8;
        upper |= param_1[2];
    }

    return static_cast<unsigned int>(last) | (upper << 8);
}
