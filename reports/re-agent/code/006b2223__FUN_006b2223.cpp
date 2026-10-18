int __cdecl FUN_006b2223(unsigned int param_1)
{
    void* object = (void*)FUN_006b18df(param_1, 1);
    FUN_006d92a1(reinterpret_cast<int>(object) + 0x28);

    int local_8;
    int entry = FUN_006b2147(object, param_1, &local_8);
    int result = 0;

    if (local_8 == 0) {
        if (entry == 0) {
            result = -3;
        } else if (*reinterpret_cast<const signed char*>(entry + 0x10) == 1) {
            result = -1;
        } else {
            result = *reinterpret_cast<const signed char*>(entry + 0x11);
        }
    }

    FUN_006d92ac(reinterpret_cast<int>(object) + 0x28);
    return result;
}