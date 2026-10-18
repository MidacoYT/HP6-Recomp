#pragma once
#include <windows.h>

#include <cstdint>
#include <string>

namespace hp6::platform {

// 0x005d7d00 (thiscall thunk, tail-jumps to 0x0100e6a0). Reads at most
// 0xff bytes of the named value under `key` into `output` and returns
// whether RegQueryValueExA succeeded. The queried value's registry type
// is read into a local and discarded (not range-checked against REG_SZ
// or REG_DWORD).
bool read_registry_value(void* output, HKEY key, LPCSTR value_name);

// 0x005d7650 (cdecl; `value_name` is received through EDI in the
// exported x86, not a stack argument — made an explicit parameter here).
// Reads the REG_SZ value `value_name` from the already-open `key` into
// `out`, retrying with a grown buffer when the first (0xff-byte) guess is
// too small. The original manipulates a std::string's internal allocator
// state directly to grow the buffer in place; growing a local
// std::string via assignment is behaviourally identical and does not
// require reproducing that internal ABI. Returns whether a value was
// read successfully.
bool read_open_key_string(std::string& out, HKEY key, const char* value_name);

// 0x005d7d40 (fastcall). Reads an integer GameSettings value from the
// registry: HKEY_CURRENT_USER first, then HKEY_LOCAL_MACHINE (promoting
// a value found there back into HKCU via write_int_setting), and
// finally creating the (empty) HKCU key when neither exists. `key_name`
// empty throws std::runtime_error, matching the exported x86's
// _CxxThrowException path (the exact message text is reconstructed from
// the visible literal fragments, not confirmed byte-for-byte — this path
// is unreachable with the fixed key names WinMain uses).
std::int32_t read_int_setting(const std::string& key_name, const std::string& section,
                              const std::string& app_name, std::int32_t default_value);

// 0x005d7810 (thiscall; `this` is key_name). Same registry fallback
// chain as read_int_setting, for a string value via read_open_key_string
// and write_string_setting; `default_value` seeds the result before the
// registry is consulted.
std::string read_string_setting(const std::string& key_name, const std::string& app_name,
                                const std::string& section,
                                const std::string& default_value);

// 0x005d7ae0 (thiscall; `this` is section, and the value is received as
// a hidden stack argument in the exported x86 rather than a named
// parameter — made explicit here). Writes a REG_DWORD GameSettings
// value under HKCU, creating the key path if needed. `key_name` empty
// throws std::runtime_error (see read_int_setting's note on the message
// text).
void write_int_setting(const std::string& key_name, const std::string& app_name,
                       const std::string& section, std::int32_t value);

// 0x005d73c0 (thiscall; `this` is section). Writes a REG_SZ GameSettings
// value (including the terminating NUL, matching the exported x86's
// length+1) under HKCU, creating the key path if needed. `key_name`
// empty throws std::runtime_error (see read_int_setting's note).
void write_string_setting(const std::string& key_name, const std::string& app_name,
                          const std::string& section, const std::string& value);

// 0x005d8080 (thunk to 0x0100e6e0, the secondary module). Saves the
// current OptionResolution, OptionLOD and OptionBrightness GameSettings
// back to the registry (HKCU\Software\Electronic Arts\Harry Potter and
// the Half Blood Prince\GameSettings), reached from WinMain's shutdown
// path. OptionLOD is DAT_008663bc, or DAT_008663b8 when the former is
// -1 (its "unset" sentinel).
void save_display_options();

// 0x00ff8300 (cdecl). Converts a registry string setting to an int via
// std::istringstream::operator>>, the same mechanism WinMain uses for
// PosX/PosY/SizeX/SizeY after reading them as strings through
// read_string_setting.
std::int32_t parse_setting_string_to_int(const std::string& value);

} // namespace hp6::platform
