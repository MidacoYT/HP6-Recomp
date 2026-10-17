#include "archive/Header.h"

namespace hp6::archive {

// 006dc532. Big-endian reader; the WORD-sized CONCAT pieces in the imported
// decompile place the earliest byte most significant, matching the "BIGF"
// magic read from 006dc5f9.
std::uint32_t read_partial_uint(const std::uint8_t* bytes, int count) {
    if (count == 1) {
        return bytes[0];
    }
    if (count == 2) {
        return static_cast<std::uint32_t>(bytes[0]) << 8 |
               static_cast<std::uint32_t>(bytes[1]);
    }
    if (count == 3) {
        const std::uint32_t high = static_cast<std::uint32_t>(bytes[0]) << 16 |
                                   static_cast<std::uint32_t>(bytes[1]) << 8;
        return static_cast<std::uint32_t>(bytes[2]) | high;
    }
    if (count == 4) {
        const std::uint32_t high = static_cast<std::uint32_t>(bytes[0]) << 24 |
                                   static_cast<std::uint32_t>(bytes[1]) << 16 |
                                   static_cast<std::uint32_t>(bytes[2]) << 8;
        return static_cast<std::uint32_t>(bytes[3]) | high;
    }
    return 0;
}

// 006dc594. The decompiled pair (uVar3, uVar2) equals one 64-bit accumulator
// shifted by eight bits per byte; bytes beyond eight shift out both here and
// in the original EDX:EAX pair.
std::uint64_t pack_bytes_be(const std::uint8_t* bytes, int count) {
    std::uint64_t packed = 0;
    for (int i = 0; i < count; ++i) {
        packed = packed << 8 | static_cast<std::uint64_t>(bytes[i]);
    }
    return packed;
}

// 006dc5f9. Classification order and repeated reads follow the decompile
// literally; the second and third checks each call the 4-byte reader again.
std::uint32_t classify_container(const std::uint8_t* bytes) {
    // The 16-bit marker is compared as a sign-extended short: 0xC0FB.
    const std::int16_t marker =
        static_cast<std::int16_t>(static_cast<std::uint16_t>(
            static_cast<std::uint32_t>(bytes[0]) << 8 |
            static_cast<std::uint32_t>(bytes[1])));
    if (marker == static_cast<std::int16_t>(-0x3f05)) {
        return 1;
    }
    if (read_partial_uint(bytes, 4) == 0x42494746u) { // "BIGF"
        return 2;
    }
    if ((read_partial_uint(bytes, 4) & 0xffffff00u) == 0x42494700u) { // "BIG?"
        return 3;
    }
    if ((read_partial_uint(bytes, 4) & 0xffffff00u) == 0x53687000u) { // "Sh??"
        return 2;
    }
    return 0;
}

// 006dc664.
std::uint32_t header_payload_offset(const std::uint8_t* bytes) {
    const std::uint32_t type = classify_container(bytes);
    if (type == 1) {
        const std::uint32_t field =
            static_cast<std::uint32_t>(bytes[2]) << 8 |
            static_cast<std::uint32_t>(bytes[3]);
        return field + 4;
    }
    if (1 < type && type < 4) {
        return read_partial_uint(bytes + 0xc, 4);
    }
    return 0;
}

// 006dc6a3. The base pointer keeps the decompiled unsigned arithmetic:
// base = bytes + (header_size - 8), wrapping on 32 bits if the size is < 8.
std::uint32_t extract_numeric_tag(const std::uint8_t* bytes,
                                  std::int32_t* digits_value,
                                  std::uint32_t* tag_value) {
    const std::uint32_t header_size = header_payload_offset(bytes);
    const auto* base = bytes +
        static_cast<std::uint32_t>(header_size - 8u);

    const auto letter = static_cast<char>(base[0]);
    if (!((letter > '@' && letter < '[') ||
          (letter > '`' && letter < '{'))) {
        return 0;
    }
    const auto second = static_cast<char>(base[1]);
    if (!(second > '/' && second < ':')) {
        return 0;
    }
    const auto third = static_cast<char>(base[2]);
    if (!(third > '/' && third < ':')) {
        return 0;
    }
    const auto fourth = static_cast<char>(base[3]);
    if (!(fourth > '/' && fourth < ':')) {
        return 0;
    }

    if (digits_value != nullptr) {
        *digits_value = static_cast<std::int32_t>(fourth) + -0x14d0 +
            ((static_cast<std::int32_t>(third) +
              static_cast<std::int32_t>(second) * 10) * 10);
    }
    if (tag_value != nullptr) {
        *tag_value = read_partial_uint(base + 4, 4);
    }
    return 8;
}

} // namespace hp6::archive
