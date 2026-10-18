// The concrete class and exception type remain unresolved. This function uses
// the binary's 32-bit std::string layout: length at +0x14, capacity at +0x18,
// and character storage at +0x04.
void __thiscall FUN_005d7ae0(
    void* this_,
    const std::string* section,
    const std::string* entry,
    DWORD value)
{
    const auto* entryBytes = reinterpret_cast<const std::byte*>(entry);
    const auto entryLength =
        *reinterpret_cast<const std::uint32_t*>(entryBytes + 0x14);

    if (entryLength == 0) {
        const std::string separator("'; entry '");
        const std::string prefix("Bad data to write to registry - section '");
        const std::string part1 = prefix + *section;
        const std::string part2 = part1 + separator;
        const std::string part3 = part2 + *entry;
        const std::string message = part3 + "'";

        alignas(4) std::byte exceptionStorage[40];
        FUN_005d6d80(
            reinterpret_cast<std::exception*>(exceptionStorage),
            &message);
        *reinterpret_cast<void**>(exceptionStorage) =
            &PTR_FUN_0083e024;
        _CxxThrowException(
            exceptionStorage,
            reinterpret_cast<ThrowInfo*>(&pThrowInfo_00848b60));
    }

    std::string key("Software\\Electronic Arts\\");
    key += *section;

    const auto* thisBytes = reinterpret_cast<const std::byte*>(this_);
    if (*reinterpret_cast<const std::uint32_t*>(thisBytes + 0x14) != 0) {
        const std::string separator("\\");
        const std::string suffix =
            separator + *reinterpret_cast<const std::string*>(this_);
        key += suffix;
    }

    HKEY keyHandle = nullptr;
    DWORD disposition = 0;
    if (RegCreateKeyExA(
            HKEY_CURRENT_USER, key.c_str(), 0, "", 0, 0x1f,
            nullptr, &keyHandle, &disposition) == ERROR_SUCCESS) {
        const char* valueName =
            *reinterpret_cast<const std::uint32_t*>(entryBytes + 0x18) < 0x10
                ? reinterpret_cast<const char*>(entryBytes + 4)
                : *reinterpret_cast<const char* const*>(entryBytes + 4);

        RegSetValueExA(
            keyHandle, valueName, 0, REG_DWORD,
            reinterpret_cast<const BYTE*>(&value), 4);
    }

    if (keyHandle != nullptr)
        RegCloseKey(keyHandle);
}