std::string* __thiscall FUN_005d7810(
    void* self,
    std::string* result,
    const std::string* section,
    void* subsection,
    const std::string* defaultValue)
{
    // Both void* arguments have the observed 32-bit std::string layout:
    // size is at +0x14. Their concrete declared types remain unresolved.
    const auto* entry = static_cast<const std::string*>(self);
    const auto* subkey = static_cast<const std::string*>(subsection);

    if (entry->size() == 0) {
        const std::string sectionAndEntry =
            *section + "'; entry '" + *entry;
        const std::string message =
            std::string("Bad registry location - section '") +
            sectionAndEntry + "'";

        // The thrown class is unresolved. The binary constructs it with
        // FUN_005d6d80, installs its vtable, then calls _CxxThrowException
        // with pThrowInfo_00848b60.
        alignas(4) std::byte exceptionStorage[40];
        FUN_005d6d80(
            reinterpret_cast<exception*>(exceptionStorage), &message);
        *reinterpret_cast<void**>(exceptionStorage) =
            &PTR_FUN_0083e024;
        _CxxThrowException(
            exceptionStorage,
            reinterpret_cast<ThrowInfo*>(&pThrowInfo_00848b60));
    }

    std::string path("Software\\Electronic Arts\\");
    path += *section;
    if (subkey->size() != 0) {
        const std::string separator("\\");
        const std::string suffix = separator + *subkey;
        path += suffix;
    }

    new (result) std::string(*defaultValue);

    HKEY key = nullptr;
    if (RegOpenKeyExA(
            HKEY_CURRENT_USER, path.c_str(), 0, 0x20019, &key) == ERROR_SUCCESS) {
        // FUN_005d7650 also reads the entry name through EDI; that implicit
        // register argument is absent from its recovered signature.
        FUN_005d7650(result, key);
    } else if (RegOpenKeyExA(
                   HKEY_LOCAL_MACHINE, path.c_str(), 0, 0x20019, &key)
               == ERROR_SUCCESS) {
        FUN_005d7650(result, key);
        FUN_005d73c0(subsection, section, self, result);
    } else {
        DWORD disposition = 0;
        RegCreateKeyExA(
            HKEY_CURRENT_USER, path.c_str(), 0, "", 0, 0x1f,
            nullptr, &key, &disposition);
    }

    if (key != nullptr)
        RegCloseKey(key);

    return result;
}
