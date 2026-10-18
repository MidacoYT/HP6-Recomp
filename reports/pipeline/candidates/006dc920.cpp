unsigned char* __cdecl FUN_006dc920(
    unsigned char* param_1, int param_2, unsigned char* param_3)
{
    unsigned char* source =
        FUN_006dc751(param_1, 0, param_2, nullptr, nullptr);

    if (source != nullptr) {
        unsigned char* destination = param_3;
        unsigned char value;
        do {
            value = *source;
            *destination = value;
            ++source;
            ++destination;
        } while (value != 0);

        return param_3;
    }

    *param_3 = 0;
    return nullptr;
}
