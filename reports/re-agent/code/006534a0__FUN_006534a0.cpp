void __cdecl FUN_006534a0(int param_1, int* param_2)
{
    if (param_1 == 0) {
        if (param_2 != nullptr) {
            (**reinterpret_cast<void (***)(int*)>(*param_2 + 8))(param_2);
        }
    } else if (param_2 != nullptr) {
        (**reinterpret_cast<void (***)(int*)>(*param_2 + 8))(param_2);
    }
}