void __fastcall FUN_006b2cbd(int* param_1)
{
    const int node = param_1[0];
    if (node == 0) {
        return;
    }

    if (node == param_1[1]) {
        param_1[1] = 0;
        param_1[0] = 0;
    } else {
        const int link = *reinterpret_cast<int*>(node + 8);
        param_1[0] = (link == 0) ? 0 : link - 8;
    }

    param_1[2] = param_1[2] - 1;
    *reinterpret_cast<int*>(node + 8) = 0;
}
