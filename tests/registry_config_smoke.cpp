// Local behavior check for batch registry_settings_055: a real registry
// round trip under a disposable HKCU test subkey, cleaned up afterward.
#include "platform/RegistryConfig.h"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>

std::int32_t DAT_00cf19a4 = 0;
std::int32_t DAT_008663bc = 0;
std::int32_t DAT_008663b8 = 0;
std::int32_t DAT_008663c0 = 0;

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}
} // namespace

int main() {
    using namespace hp6::platform;

    const char* subkey = "Software\\HP6Reconstructed_Test";
    HKEY key = nullptr;
    LONG created = RegCreateKeyExA(HKEY_CURRENT_USER, subkey, 0, nullptr, 0,
                                   KEY_ALL_ACCESS, nullptr, &key, nullptr);
    expect("key_created", created == ERROR_SUCCESS);

    const char value[] = "1024";
    RegSetValueExA(key, "Width", 0, REG_SZ, reinterpret_cast<const BYTE*>(value),
                   static_cast<DWORD>(sizeof value));

    char buffer[0xff] = {};
    const bool found = read_registry_value(buffer, key, "Width");
    expect("value_found", found);
    expect("value_matches", std::strcmp(buffer, "1024") == 0);

    char missing[0xff] = {};
    const bool absent = read_registry_value(missing, key, "DoesNotExist");
    expect("missing_value", !absent);

    // ---- read_open_key_string (005d7650) -----------------------------------
    std::string small_value;
    expect("open_key_string_small", read_open_key_string(small_value, key, "Width") &&
                                    small_value == "1024");

    std::string long_literal(400, 'x'); // forces the ERROR_MORE_DATA grow path (> 0xff)
    RegSetValueExA(key, "Long", 0, REG_SZ,
                   reinterpret_cast<const BYTE*>(long_literal.c_str()),
                   static_cast<DWORD>(long_literal.size() + 1));
    std::string grown_value;
    expect("open_key_string_grown", read_open_key_string(grown_value, key, "Long") &&
                                    grown_value == long_literal);

    std::string missing_value;
    expect("open_key_string_missing", !read_open_key_string(missing_value, key, "NoSuchValue"));

    RegCloseKey(key);
    RegDeleteKeyA(HKEY_CURRENT_USER, subkey);

    // ---- read_int_setting / write_int_setting (005d7d40 / 005d7ae0) -------
    // write_int_setting/write_string_setting always target
    // HKCU\Software\Electronic Arts\<app_name>[\<section>], matching the
    // exported x86; a disposable app_name keeps this out of the way of any
    // real EA game installation.
    const std::string app_name = "HP6Reconstructed_Test";
    const std::string section = "GameSettings";
    const std::string settings_root =
        "Software\\Electronic Arts\\" + app_name;

    expect("int_default_when_absent",
           read_int_setting("Width", section, app_name, 640) == 640);

    write_int_setting("Width", app_name, section, 1280);
    expect("int_round_trip", read_int_setting("Width", section, app_name, 0) == 1280);

    // ---- read_string_setting / write_string_setting (005d7810 / 005d73c0) -
    expect("string_default_when_absent",
           read_string_setting("Mode", app_name, section, "Fallback") == "Fallback");

    write_string_setting("Mode", app_name, section, "Fullscreen");
    expect("string_round_trip",
           read_string_setting("Mode", app_name, section, "") == "Fullscreen");

    // ---- empty key name: std::runtime_error -------------------------------
    bool threw = false;
    try {
        read_int_setting("", section, app_name, 0);
    } catch (const std::runtime_error&) {
        threw = true;
    }
    expect("empty_key_name_throws", threw);

    // Best-effort cleanup of only the keys this test created; "Software\
    // Electronic Arts" itself is left alone in case a real EA game's
    // settings already live there.
    RegDeleteKeyA(HKEY_CURRENT_USER,
                 (settings_root + "\\" + section).c_str());
    RegDeleteKeyA(HKEY_CURRENT_USER, settings_root.c_str());

    // ---- save_display_options (0x0100e6e0, thunk 0x005d8080) --------------
    // This one writes to the real game's registry path (the app name is
    // compiled in, not a parameter), so any value already there for
    // OptionResolution/OptionLOD/OptionBrightness is saved first and
    // restored afterward, leaving no net change.
    {
        const char* real_key =
            "Software\\Electronic Arts\\Harry Potter and the Half Blood Prince\\"
            "GameSettings";
        HKEY existing = nullptr;
        const bool key_existed =
            RegOpenKeyExA(HKEY_CURRENT_USER, real_key, 0, KEY_ALL_ACCESS, &existing) ==
            ERROR_SUCCESS;
        std::int32_t saved_resolution = 0, saved_lod = 0, saved_brightness = 0;
        bool had_resolution = false, had_lod = false, had_brightness = false;
        if (key_existed) {
            had_resolution = read_registry_value(&saved_resolution, existing, "OptionResolution");
            had_lod = read_registry_value(&saved_lod, existing, "OptionLOD");
            had_brightness = read_registry_value(&saved_brightness, existing, "OptionBrightness");
        }

        DAT_00cf19a4 = 7;
        DAT_008663bc = -1; // "unset": falls back to DAT_008663b8
        DAT_008663b8 = 3;
        DAT_008663c0 = 9;
        save_display_options();

        HKEY written = nullptr;
        const bool opened_after =
            RegOpenKeyExA(HKEY_CURRENT_USER, real_key, 0, KEY_ALL_ACCESS, &written) ==
            ERROR_SUCCESS;
        expect("display_options_key_written", opened_after);
        if (opened_after) {
            std::int32_t resolution = 0, lod = 0, brightness = 0;
            expect("display_options_resolution",
                   read_registry_value(&resolution, written, "OptionResolution") &&
                       resolution == 7);
            expect("display_options_lod_fallback",
                   read_registry_value(&lod, written, "OptionLOD") && lod == 3);
            expect("display_options_brightness",
                   read_registry_value(&brightness, written, "OptionBrightness") &&
                       brightness == 9);

            // Restore whatever was there before (or remove the values this
            // test added, if there was nothing before).
            if (had_resolution) {
                RegSetValueExA(written, "OptionResolution", 0, REG_DWORD,
                               reinterpret_cast<const BYTE*>(&saved_resolution), 4);
            } else {
                RegDeleteValueA(written, "OptionResolution");
            }
            if (had_lod) {
                RegSetValueExA(written, "OptionLOD", 0, REG_DWORD,
                               reinterpret_cast<const BYTE*>(&saved_lod), 4);
            } else {
                RegDeleteValueA(written, "OptionLOD");
            }
            if (had_brightness) {
                RegSetValueExA(written, "OptionBrightness", 0, REG_DWORD,
                               reinterpret_cast<const BYTE*>(&saved_brightness), 4);
            } else {
                RegDeleteValueA(written, "OptionBrightness");
            }
            RegCloseKey(written);
        }
        if (existing != nullptr) {
            RegCloseKey(existing);
        }
        if (!key_existed) {
            // The key did not exist before this test; remove it only if
            // save_display_options is all that created it (best effort).
            RegDeleteKeyA(HKEY_CURRENT_USER, real_key);
        }
    }

    if (failures != 0) {
        std::printf("registry_config_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("registry_config_smoke: all checks passed\n");
    return 0;
}
