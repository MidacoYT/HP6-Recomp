// The owning class and the exception type are unresolved. The string arguments
// use the 32-bit MSVC std::string layout: size at +0x14 and capacity at +0x18.
struct Class_005d73c0 {
    void __thiscall FUN_005d73c0(
        const std::string& section,
        const std::string& entry,
        const std::string& value);
};

void __thiscall Class_005d73c0::FUN_005d73c0(
    const std::string& section,
    const std::string& entry,
    const std::string& value)
{
    HKEY key = nullptr;
    DWORD disposition = 0;

    if (entry.empty()) {
        const std::string message =
            std::string("Bad data to write to registry - section '")
            + section + "'; entry '" + entry
            + "' value '" + value + "'";

        throw Exception_005d6d80(message); // Type/vtable at 0x0083e024.
    }

    std::string path("Software\\Electronic Arts\\");
    path += section;

    // The object at ECX is used as an MSVC std::string; its owning type is unknown.
    const auto& suffix = *reinterpret_cast<const std::string*>(this);
    if (!suffix.empty()) {
        path += std::string("\\") + suffix;
    }

    if (RegCreateKeyExA(
            HKEY_CURRENT_USER, path.c_str(), 0, "", 0, 0x1f,
            nullptr, &key, &disposition) == ERROR_SUCCESS) {
        RegSetValueExA(
            key, entry.c_str(), 0, REG_SZ,
            reinterpret_cast<const BYTE*>(value.c_str()),
            static_cast<DWORD>(value.size() + 1));
    }

    if (key != nullptr) {
        RegCloseKey(key);
    }
}
