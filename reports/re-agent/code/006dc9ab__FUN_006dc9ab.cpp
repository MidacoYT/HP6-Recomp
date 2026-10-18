void __cdecl FUN_006dc9ab(unsigned char* param_1, char* param_2,
                           unsigned char** param_3)
{
    while (true) {
        char* comma = strchr(param_2, ',');
        if (comma != nullptr) {
            *comma = '\0';
        }

        *param_3 = FUN_006dc989(param_1, reinterpret_cast<int>(param_2));
        ++param_3;

        if (comma == nullptr) {
            break;
        }

        *comma = ',';
        param_2 = comma + 1;
        if (param_2 == nullptr) {
            return;
        }
    }
}