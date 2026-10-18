extern DWORD _DAT_00899acc;

bool __cdecl FUN_006d944e(int param_1)
{
    if (param_1 == 0) {
        return GetCurrentThreadId() == _DAT_00899acc;
    }

    if (param_1 == -1) {
        return true;
    }

    const DWORD currentThreadId = GetCurrentThreadId();
    return *reinterpret_cast<const DWORD*>(param_1 + 8) == currentThreadId;
}
