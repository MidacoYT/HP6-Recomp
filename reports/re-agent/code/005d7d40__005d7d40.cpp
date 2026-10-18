DWORD __fastcall FUN_005d7d40(
    std::string* param_1,
    std::string* param_2,
    std::string* param_3,
    DWORD param_4)
{
    if (param_1->empty()) {
        std::string sectionAndEntry = *param_2 + "'; entry '";
        std::string withEntry = sectionAndEntry + *param_1;
        std::string prefix("Bad registry location - section '");
        std::string withPrefix = prefix + withEntry;
        std::string message = withPrefix + "'";

        // The exception type is unresolved. Preserve its construction,
        // vtable assignment, and throw helper by their known symbols.
        void* exceptionStorage[10];
        FUN_005d6d80(
            reinterpret_cast<std::exception*>(exceptionStorage),
            &message);
        exceptionStorage[0] = reinterpret_cast<void*>(0x0083e024);
        _CxxThrowException(
            exceptionStorage,
            reinterpret_cast<ThrowInfo*>(0x00848b60));
    }

    std::string path("Software\\Electronic Arts\\");
    path += *param_3;

    if (!param_2->empty()) {
        std::string separator = this_0082b204; // Contains "\\"
        std::string suffix = separator + *param_2;
        path += suffix;
    }

    HKEY key = nullptr;
    DWORD result = param_4;

    if (RegOpenKeyExA(
            HKEY_CURRENT_USER, path.c_str(), 0, 0x20019, &key) == ERROR_SUCCESS) {
        DWORD valueType = REG_SZ;
        DWORD dataSize = 0xff;
        RegQueryValueExA(
            key, param_1->c_str(), nullptr, &valueType,
            reinterpret_cast<LPBYTE>(&result), &dataSize);
    } else if (RegOpenKeyExA(
                   HKEY_LOCAL_MACHINE, path.c_str(), 0, 0x20019,
                   &key) == ERROR_SUCCESS) {
        thunk_FUN_0100e6a0(&result, key, param_1->c_str());
        FUN_005d7ae0(param_2, param_3, param_1);
    } else {
        DWORD disposition = 0;
        RegCreateKeyExA(
            HKEY_CURRENT_USER, path.c_str(), 0, "", 0, 0x1f,
            nullptr, &key, &disposition);
    }

    if (key != nullptr) {
        RegCloseKey(key);
    }

    return result;
}