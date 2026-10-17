#include "archive/Entry.h"
#include "archive/Header.h"
#include <cctype>
#include <cstddef>
#include <cstring>

namespace hp6::archive {

// 006dc5c9. The decompile's loop compares lowered bytes until a mismatch or
// terminator, then returns the signed difference of the last processed pair.
std::int32_t case_insensitive_compare(const char* left, const char* right) {
    while (true) {
        const std::int32_t first = std::tolower(static_cast<int>(*left));
        const std::int32_t second = std::tolower(static_cast<int>(*right));
        if ((first - second) != 0 || *left == '\0') {
            return first - second;
        }
        ++right;
        ++left;
    }
}

// 006dc751. Control flow, cursor bases (0x10 / 6), width derivation
// (byte 3 minus 0x30 clamped at 4), entry cursor advance over the NUL
// terminator and the double-width name pointer all follow the exported
// assembly sequence at 0x006dc751..0x006dc84f.
const std::uint8_t* find_entry(const std::uint8_t* bytes,
                               const char* requested, int index,
                               std::int32_t* field_value,
                               std::uint32_t* raw_value) {
    const auto* base = bytes;
    std::int32_t seen = 0;
    const std::int32_t tag_size = extract_numeric_tag(bytes, nullptr, nullptr);
    std::int32_t width = 4;
    const auto* cursor = bytes + 0x10;
    const std::uint32_t payload_offset = header_payload_offset(bytes);
    const auto* finish = bytes +
        static_cast<std::uint32_t>(
            payload_offset - static_cast<std::uint32_t>(tag_size));

    const std::uint32_t container = classify_container(bytes);
    if (container == 1) {
        width = 3;
        cursor = bytes + 6;
    } else if (container == 3) {
        width = static_cast<std::int32_t>(
            static_cast<char>(base[3])) - 0x30;
        if (4 < width) {
            width = 4;
        }
    }

    const auto* result = static_cast<const std::uint8_t*>(nullptr);
    if (requested == nullptr) {
        if (cursor < finish) {
            do {
                result = cursor + static_cast<std::size_t>(width) +
                    static_cast<std::size_t>(width);
                if (seen == index) {
                    goto found;
                }
                while (*result++ != 0) {
                }
                ++seen;
                cursor = result;
            } while (result < finish);
        }
    } else {
        while (cursor < finish) {
            result = cursor + static_cast<std::size_t>(width) +
                static_cast<std::size_t>(width);
            if (case_insensitive_compare(
                    reinterpret_cast<const char*>(result), requested) == 0) {
                goto found;
            }
            while (*result++ != 0) {
            }
            cursor = result;
        }
    }

    if (field_value != nullptr) {
        *field_value = 0;
    }
    if (raw_value != nullptr) {
        *raw_value = 0;
    }
    return nullptr;

found:
    if (field_value != nullptr) {
        *field_value = static_cast<std::int32_t>(
            static_cast<std::uint32_t>(
                pack_bytes_be(cursor, width) & 0xffffffffu));
    }
    if (raw_value == nullptr) {
        return result;
    }
    *raw_value = read_partial_uint(cursor + width, width);
    return result;
}

std::int32_t entry_field_by_name(const std::uint8_t* bytes,
                                 const char* requested) {
    std::int32_t value = 0;
    if (requested != nullptr) {
        find_entry(bytes, requested, 0, &value, nullptr);
    }
    return value;
}

std::int32_t entry_field_by_name_twin(const std::uint8_t* bytes,
                                      const char* requested) {
    std::int32_t value = 0;
    if (requested != nullptr) {
        find_entry(bytes, requested, 0, &value, nullptr);
    }
    return value;
}

const std::uint8_t* entry_pointer_by_index(const std::uint8_t* bytes,
                                           int index) {
    std::int32_t value = 0;
    const auto* result = find_entry(bytes, nullptr, index, &value, nullptr);
    return result == nullptr ? nullptr : bytes + value;
}

namespace {

// Shared by both copy wrappers (006dc920 / 006dc9eb): byte copy of the
// internal name string including its NUL, struck through the source/destination
// distance exactly as decompiled.
char* copy_entry_string_impl(const std::uint8_t* bytes, int index,
                             char* output) {
    const auto* source = find_entry(bytes, nullptr, index, nullptr, nullptr);
    if (source == nullptr) {
        *output = '\0';
        return nullptr;
    }
    const auto delta =
        static_cast<std::size_t>(reinterpret_cast<std::uintptr_t>(output) -
                                 reinterpret_cast<std::uintptr_t>(source));
    auto* destination = const_cast<std::uint8_t*>(
        source + static_cast<std::intptr_t>(delta));
    for (std::uint8_t current = 0;; ++source) {
        current = *source;
        *(destination++) = current;
        if (current == 0) {
            break;
        }
    }
    return output;
}

} // namespace

char* copy_entry_string(const std::uint8_t* bytes, int index, char* output) {
    return copy_entry_string_impl(bytes, index, output);
}

char* copy_entry_string_twin(const std::uint8_t* bytes, int index,
                             char* output) {
    return copy_entry_string_impl(bytes, index, output);
}

void entry_forward(const std::uint8_t* bytes, const char* requested,
                   int index, std::int32_t* field_value,
                   std::uint32_t* raw_value) {
    find_entry(bytes, requested, index, field_value, raw_value);
}

// 006dc733. The call discards the tag value; only the digits sum survives.
std::int32_t tag_digits_value(const std::uint8_t* bytes) {
    std::int32_t digits = 0;
    extract_numeric_tag(bytes, &digits, nullptr);
    return digits;
}

// 006dc8e4.
std::uint32_t header_field_32(const std::uint8_t* bytes) {
    const std::uint32_t container = classify_container(bytes);
    if (container == 1) {
        return static_cast<std::uint32_t>(static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(bytes[4]) << 8 |
            static_cast<std::uint32_t>(bytes[5])));
    }
    if (1 < container && container < 4) {
        return read_partial_uint(bytes + 8, 4);
    }
    return 0;
}

// 006dc8c2 / 006dc989 — byte-identical bodies at distinct addresses.
const std::uint8_t* entry_pointer_by_name(const std::uint8_t* bytes,
                                          const char* requested) {
    // 006dc8c2 resolves through FUN_006dc86f (the non-twin value wrapper).
    const std::int32_t value = entry_field_by_name(bytes, requested);
    if (value != 0) {
        return bytes + value;
    }
    return nullptr;
}

const std::uint8_t* entry_pointer_by_name_twin(const std::uint8_t* bytes,
                                               const char* requested) {
    // 006dc989 resolves through FUN_006dc962 (the twin value wrapper).
    const std::int32_t value = entry_field_by_name_twin(bytes, requested);
    if (value != 0) {
        return bytes + value;
    }
    return nullptr;
}

// 006dc9ab. Walks names separated by ',' and nulls the comma in place for
// each lookup, restoring it before advancing; each resolved pointer (or
// nullptr from the twin) is stored in the next output slot.
void resolve_entry_list(const std::uint8_t* bytes, char* names,
                        const std::uint8_t** out_ptrs) {
    while (true) {
        char* separator = std::strchr(names, 0x2c);
        if (separator != nullptr) {
            *separator = '\0';
        }
        *out_ptrs = entry_pointer_by_name_twin(bytes, names);
        ++out_ptrs;
        if (separator == nullptr) {
            return;
        }
        *separator = ',';
        names = separator + 1;
        // Inert in the decompile (pointer arithmetic is never null); kept
        // for source fidelity with the export.
        if (names == nullptr) {
            return;
        }
    }
}

} // namespace hp6::archive
