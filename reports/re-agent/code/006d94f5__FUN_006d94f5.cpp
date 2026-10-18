std::uint32_t __cdecl FUN_006d94f5(int param_1, int param_2)
{
    HANDLE hThread = lpTargetHandle_00899ad0;

    if (param_1 != 0) {
        if (param_1 == -1) {
            hThread = GetCurrentThread();
        } else {
            hThread = *reinterpret_cast<HANDLE*>(param_1 + 4);
        }
    }

    BOOL result = 0;
    if (hThread != nullptr) {
        int priority;
        if (param_2 == -3) {
            priority = -15;
        } else if (param_2 == -2) {
            priority = -2;
        } else if (param_2 == -1) {
            priority = -1;
        } else if (param_2 == 0) {
            priority = 0;
        } else if (param_2 == 1) {
            priority = 1;
        } else if (param_2 == 2) {
            priority = 2;
        } else if (param_2 == 3) {
            priority = 15;
        } else {
            priority = 0;
        }

        result = SetThreadPriority(hThread, priority);
    }

    return (static_cast<std::uint32_t>(result) & 0xFFFFFF00u) | 1u;
}