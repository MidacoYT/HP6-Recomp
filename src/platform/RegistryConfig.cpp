#include "platform/RegistryConfig.h"

#include <sstream>
#include <stdexcept>
#include <vector>

// Addresses read by the binary but absent from the exported globals; the
// final binary must supply their stored values explicitly.
extern std::int32_t DAT_00cf19a4;
extern std::int32_t DAT_008663bc;
extern std::int32_t DAT_008663b8;
extern std::int32_t DAT_008663c0;

namespace hp6::platform {

namespace {
constexpr DWORD kOpenFlags = 0x20019; // KEY_READ-equivalent sam flags used by the exported x86
constexpr DWORD kCreateFlags = 0x1f;  // sam flags used by the exported RegCreateKeyExA calls

std::string game_settings_path(const std::string& app_name, const std::string& section) {
    std::string path = "Software\\Electronic Arts\\" + app_name;
    if (!section.empty()) {
        path += "\\" + section;
    }
    return path;
}
} // namespace

// ReAgent candidate: reports/pipeline/candidates/005d7d00.cpp.
bool read_registry_value(void* output, HKEY key, LPCSTR value_name) {
    DWORD data_size = 0xff;
    DWORD value_type = 1;
    return RegQueryValueExA(key, value_name, nullptr, &value_type,
                            static_cast<LPBYTE>(output), &data_size) == ERROR_SUCCESS;
}

// Not a ReAgent batch; reconstructed directly from the exported x86. The
// original grows a std::string's own internal buffer via the MSVC STL's
// allocator internals (thunk_FUN_00ff8440 / thunk_FUN_00ff84a0, tracked
// as external CRT/STL dependencies) on ERROR_MORE_DATA; a local buffer
// resized the same way is behaviourally identical.
bool read_open_key_string(std::string& out, HKEY key, const char* value_name) {
    std::vector<char> buffer(0xff);
    DWORD type = 1;
    DWORD size = static_cast<DWORD>(buffer.size());
    LSTATUS status = RegQueryValueExA(key, value_name, nullptr, &type,
                                      reinterpret_cast<LPBYTE>(buffer.data()), &size);
    if (status == ERROR_MORE_DATA) {
        buffer.assign(size, '\0');
        status = RegQueryValueExA(key, value_name, nullptr, &type,
                                  reinterpret_cast<LPBYTE>(buffer.data()), &size);
    }
    if (status != ERROR_SUCCESS) {
        return false;
    }
    out.assign(buffer.data());
    return true;
}

// Not a ReAgent batch; reconstructed directly from the exported x86
// after resolving the MSVC 2008 std::basic_string layout (a 4-byte
// iterator-debug header before the classic _Bx/_Mysize/_Myres fields,
// matching the observed +4 shift and the 28-byte object size seen
// throughout this binary's exported locals) — see
// architecture/startup_path.md. The SEH prologue/epilogue and the
// repeated string temporaries visible in the decompiled C are MSVC's
// own generated bookkeeping for local std::string destructors, not game
// logic; they have no counterpart here.
std::int32_t read_int_setting(const std::string& key_name, const std::string& section,
                              const std::string& app_name, std::int32_t default_value) {
    if (key_name.empty()) {
        throw std::runtime_error("Bad registry location - section '" + section +
                                 "'; entry '" + key_name + "'");
    }
    const std::string path = game_settings_path(app_name, section);
    std::int32_t result = default_value;
    HKEY key = nullptr;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, path.c_str(), 0, kOpenFlags, &key) == ERROR_SUCCESS) {
        read_registry_value(&result, key, key_name.c_str());
    } else if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, path.c_str(), 0, kOpenFlags, &key) ==
               ERROR_SUCCESS) {
        read_registry_value(&result, key, key_name.c_str());
        write_int_setting(key_name, app_name, section, result);
    } else {
        HKEY created = nullptr;
        DWORD disposition = 0;
        RegCreateKeyExA(HKEY_CURRENT_USER, path.c_str(), 0, const_cast<LPSTR>(""), 0,
                       kCreateFlags, nullptr, &created, &disposition);
        if (created != nullptr) {
            RegCloseKey(created);
        }
    }
    if (key != nullptr) {
        RegCloseKey(key);
    }
    return result;
}

// Not a ReAgent batch; see read_int_setting's note.
std::string read_string_setting(const std::string& key_name, const std::string& app_name,
                                const std::string& section,
                                const std::string& default_value) {
    if (key_name.empty()) {
        throw std::runtime_error("Bad registry location - section '" + section +
                                 "'; entry '" + key_name + "'");
    }
    const std::string path = game_settings_path(app_name, section);
    std::string result = default_value;
    HKEY key = nullptr;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, path.c_str(), 0, kOpenFlags, &key) == ERROR_SUCCESS) {
        read_open_key_string(result, key, key_name.c_str());
    } else if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, path.c_str(), 0, kOpenFlags, &key) ==
               ERROR_SUCCESS) {
        read_open_key_string(result, key, key_name.c_str());
        write_string_setting(key_name, app_name, section, result);
    } else {
        HKEY created = nullptr;
        DWORD disposition = 0;
        RegCreateKeyExA(HKEY_CURRENT_USER, path.c_str(), 0, const_cast<LPSTR>(""), 0,
                       kCreateFlags, nullptr, &created, &disposition);
        if (created != nullptr) {
            RegCloseKey(created);
        }
    }
    if (key != nullptr) {
        RegCloseKey(key);
    }
    return result;
}

// Not a ReAgent batch; see read_int_setting's note.
void write_int_setting(const std::string& key_name, const std::string& app_name,
                       const std::string& section, std::int32_t value) {
    if (key_name.empty()) {
        throw std::runtime_error("Bad data to write to registry - section '" + section +
                                 "'; entry '" + key_name + "'");
    }
    const std::string path = game_settings_path(app_name, section);
    HKEY key = nullptr;
    DWORD disposition = 0;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, path.c_str(), 0, const_cast<LPSTR>(""), 0,
                        kCreateFlags, nullptr, &key, &disposition) == ERROR_SUCCESS) {
        RegSetValueExA(key, key_name.c_str(), 0, REG_DWORD,
                       reinterpret_cast<const BYTE*>(&value), sizeof(value));
    }
    if (key != nullptr) {
        RegCloseKey(key);
    }
}

// Not a ReAgent batch; see read_int_setting's note.
void write_string_setting(const std::string& key_name, const std::string& app_name,
                          const std::string& section, const std::string& value) {
    if (key_name.empty()) {
        throw std::runtime_error("Bad data to write to registry - section '" + section +
                                 "'; entry '" + key_name + "'");
    }
    const std::string path = game_settings_path(app_name, section);
    HKEY key = nullptr;
    DWORD disposition = 0;
    if (RegCreateKeyExA(HKEY_CURRENT_USER, path.c_str(), 0, const_cast<LPSTR>(""), 0,
                        kCreateFlags, nullptr, &key, &disposition) == ERROR_SUCCESS) {
        RegSetValueExA(key, key_name.c_str(), 0, REG_SZ,
                       reinterpret_cast<const BYTE*>(value.c_str()),
                       static_cast<DWORD>(value.size() + 1));
    }
    if (key != nullptr) {
        RegCloseKey(key);
    }
}

// Not a ReAgent batch; reconstructed directly from the exported x86 of
// 0x0100e6e0 (0x005d8080 is a plain tail-jump thunk to it). The string
// literals and the write_int_setting calls below match the decompiled C
// exactly; the compiler-generated std::string temporary construction/
// destruction around each call (via the two IAT slots repeatedly
// invoked in the exported x86) has no counterpart here.
void save_display_options() {
    constexpr char kAppName[] = "Harry Potter and the Half Blood Prince";
    constexpr char kSection[] = "GameSettings";

    write_int_setting("OptionResolution", kAppName, kSection, DAT_00cf19a4);

    std::int32_t lod = DAT_008663bc;
    if (lod == -1) {
        lod = DAT_008663b8;
    }
    write_int_setting("OptionLOD", kAppName, kSection, lod);

    write_int_setting("OptionBrightness", kAppName, kSection, DAT_008663c0);
}

// 0x00ff8300. Converts a registry string setting (PosX, PosY, SizeX,
// SizeY in WinMain) to an int through an istringstream, exactly as the
// exported x86 does via basic_istringstream::operator>>; a malformed or
// empty string yields 0 (the stream's own default-constructed result),
// matching the original's uninitialized-but-zeroed stack behaviour for
// that case.
std::int32_t parse_setting_string_to_int(const std::string& value) {
    std::istringstream stream(value);
    std::int32_t result = 0;
    stream >> result;
    return result;
}

} // namespace hp6::platform
