unsigned char* __cdecl FUN_006dc9eb(
    unsigned char* param_1,
    int param_2,
    unsigned char* param_3)
{
    unsigned char* source = FUN_006dc751(param_1, 0, param_2, nullptr, nullptr);

    if (source != nullptr) {
        const int offset =
            reinterpret_cast<int>(param_3) - reinterpret_cast<int>(source);

        unsigned char value;
        do {
            value = *source;
            source[offset] = value;
            ++source;
        } while (value != 0);

        return param_3;
    }

    *param_3 = 0;
    return nullptr;
}
