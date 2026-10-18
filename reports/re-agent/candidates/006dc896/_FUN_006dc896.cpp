unsigned char* __cdecl FUN_006dc896(unsigned char* param_1, int param_2)
{
    int local_8;
    unsigned char* result =
        FUN_006dc751(param_1, 0, param_2, &local_8, nullptr);

    if (result == nullptr) {
        result = nullptr;
    } else {
        result = param_1 + local_8;
    }

    return result;
}
