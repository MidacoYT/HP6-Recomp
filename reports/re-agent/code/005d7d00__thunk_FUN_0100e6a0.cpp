bool __thiscall thunk_FUN_0100e6a0(
    void* output,
    HKEY param_1,
    LPCSTR param_2)
{
    DWORD dataSize = 0xff;
    DWORD valueType = 1;

    return RegQueryValueExA(
        param_1,
        param_2,
        nullptr,
        &valueType,
        static_cast<LPBYTE>(output),
        &dataSize) == 0;
}