// x86 MSVC ABI. The registry value name arrives in EDI; the supplied
// signature does not account for that register input.
bool __cdecl FUN_005d7650(std::string* param_1, HKEY param_2)
{
    const char* valueName;
    __asm { mov valueName, edi }

    std::vector<char> buffer(0xff, '\0');
    DWORD size = 0xff;
    DWORD type = 1;

    if (buffer.data() == nullptr || buffer.empty())
        _invalid_parameter_noinfo();

    LSTATUS status = RegQueryValueExA(
        param_2, valueName, nullptr, &type,
        reinterpret_cast<LPBYTE>(buffer.data()), &size);

    if (status != ERROR_SUCCESS) {
        if (status != ERROR_MORE_DATA || buffer.size() >= size)
            return false;

        buffer.resize(size);

        if (buffer.data() == nullptr || buffer.empty())
            _invalid_parameter_noinfo();

        status = RegQueryValueExA(
            param_2, valueName, nullptr, &type,
            reinterpret_cast<LPBYTE>(buffer.data()), &size);

        if (status != ERROR_SUCCESS)
            return false;
    }

    if (buffer.data() == nullptr || buffer.empty())
        _invalid_parameter_noinfo();

    std::string value(buffer.data());
    *param_1 = value;
    return true;
}
