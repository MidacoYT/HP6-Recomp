#include "projection/RecordDecode.h"
#include "projection/RecordFields.h"
#include "projection/RecordFlags.h"
#include "projection/FormatSize.h"
#include <cstddef>
#include <cstdio>
#include <cstring>

namespace hp6::projection {


static std::uint32_t read_word(const void* record, std::size_t offset) {
    std::uint32_t word;
    std::memcpy(&word, static_cast<const std::byte*>(record) + offset, sizeof word);
    return word;
}

static std::uint16_t read_half(const void* record, std::size_t offset) {
    std::uint16_t value;
    std::memcpy(&value, static_cast<const std::byte*>(record) + offset, sizeof value);
    return value;
}

static void write_word(void* record, std::size_t offset, std::uint32_t word) {
    std::memcpy(static_cast<std::byte*>(record) + offset, &word, sizeof word);
}

static void copy_relative_payload(const void* source, void* destination) {
    const std::uint32_t source_offset = read_word(source, 0x08);
    const std::uint32_t destination_offset = read_word(destination, 0x08);
    const auto* input_data = source_offset == 0 ? nullptr :
        static_cast<const std::byte*>(source) + static_cast<std::int32_t>(source_offset);
    auto* output_data = destination_offset == 0 ? nullptr :
        static_cast<std::byte*>(destination) + static_cast<std::int32_t>(destination_offset);
    const std::uint32_t count = read_word(source, 0x0c);
    for (std::uint32_t offset = 0; offset < (count & ~3u); offset += 4) {
        std::uint32_t word;
        std::memcpy(&word, input_data + offset, sizeof word);
        std::memcpy(output_data + offset, &word, sizeof word);
    }
    for (std::uint32_t offset = count & ~3u; offset < count; ++offset) {
        output_data[offset] = input_data[offset];
    }
}

void set_relative_payload(void* record, const void* payload, std::uint32_t size) {
    if (payload == nullptr) {
        write_word(record, 0x08, 0);
        write_word(record, 0x0c, 0);
        return;
    }
    const auto relative = reinterpret_cast<std::uintptr_t>(payload) -
                          reinterpret_cast<std::uintptr_t>(record);
    write_word(record, 0x08, static_cast<std::uint32_t>(relative));
    write_word(record, 0x0c, size);
}

void link_next_record(void* record, const void* next) {
    const auto relative = next == nullptr ? 0u : static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(next) - reinterpret_cast<std::uintptr_t>(record));
    write_word(record, 0x04, relative);
}

void decode_variant_02(const void* source, void* destination) {
    initialize_record_header(destination, source);
    auto* output = static_cast<std::byte*>(destination);
    output[0] = static_cast<const std::byte*>(source)[0x14];
    write_word(destination, 0x18, read_half(source, 0x18));
    write_word(destination, 0x1c, read_half(source, 0x1c));
    write_word(destination, 0x10, read_word(source, 0x20));
    write_word(destination, 0x14, read_word(source, 0x24));
    write_word(destination, 0, (read_word(source, 0x28) << 28) |
                               (read_word(destination, 0) & 0x0fffffffu));
    set_flag_24(destination, static_cast<std::int32_t>(read_word(source, 0x30)));
    if (!is_selected_format_code(read_word(source, 0x14))) {
        output[2] &= std::byte{0xbf};
    } else {
        set_flag_22(destination, static_cast<std::int32_t>(read_word(source, 0x2c)));
    }
    set_flag_16(destination, static_cast<std::int32_t>(read_word(source, 0x34)));
    set_flag_17(destination, static_cast<std::int32_t>(read_word(source, 0x38)));
    set_flag_18(destination, static_cast<std::int32_t>(read_word(source, 0x3c)));
    set_flag_19(destination, static_cast<std::int32_t>(read_word(source, 0x40)));
    set_flag_20(destination, static_cast<std::int32_t>(read_word(source, 0x44)));
    set_flag_21(destination, static_cast<std::int32_t>(read_word(source, 0x48)));
    set_flag_23(destination, static_cast<std::int32_t>(read_word(source, 0x4c)));
    const std::uint32_t size = static_cast<std::uint32_t>(estimate_record_size(source)) - 0x20u;
    set_relative_payload(destination, output + 0x20, size);
}

void decode_variant_100(const void* source, void* destination) {
    initialize_record_header(destination, source);
    auto* output = static_cast<std::byte*>(destination);
    output[0] = std::byte{0x7a};
    write_word(destination, 0x10, read_word(source, 0x64));
    write_word(destination, 0x14, read_word(source, 0x68));
    write_word(destination, 0x18, read_word(source, 0x6c));
    write_word(destination, 0x1c, read_word(source, 0x70));
    set_relative_payload(destination, output + 0x20, 0);
}

void decode_variant_20(const void* source, void* destination) {
    initialize_record_header(destination, source);
    auto* output = static_cast<std::byte*>(destination);
    output[0] = std::byte{0x6f};
    set_relative_payload(destination, output + 0x10, read_word(source, 0x90) + 1u);
}

void decode_variant_04(const void* source, void* destination) {
    initialize_record_header(destination, source);
    auto* output = static_cast<std::byte*>(destination);
    output[0] = static_cast<const std::byte*>(source)[0x54];
    write_word(destination, 0x18, read_half(source, 0x5c));
    write_word(destination, 0x1c, 1);
    set_flag_22(destination, static_cast<std::int32_t>(read_word(source, 0x58)));
    write_word(destination, 0, read_word(destination, 0) & 0x0effffffu);
    const std::uint32_t size = static_cast<std::uint32_t>(estimate_variant_04_size(source)) - 0x20u;
    set_relative_payload(destination, output + 0x20, size);
}

void decode_variant_10(const void* source, void* destination) {
    initialize_record_header(destination, source);
    auto* output = static_cast<std::byte*>(destination);
    output[0] = std::byte{0x7c};
    write_word(destination, 0x10, read_word(source, 0x74));
    write_word(destination, 0x14, read_word(source, 0x78));
    write_word(destination, 0x18, read_word(source, 0x7c));
    write_word(destination, 0x1c, read_word(source, 0x80));
    const std::uint32_t size = static_cast<std::uint32_t>(estimate_variant_10_size(source)) - 0x20u;
    set_relative_payload(destination, output + 0x20, size);
}

void clone_variant_02(const void* source, void* destination) {
    std::byte projected[0xc0];
    initialize_record_and_return(projected);
    write_variant_02(source, projected);
    decode_variant_02(projected, destination);
    auto* output = static_cast<std::byte*>(destination);
    output[1] &= std::byte{0xfb};
    if (read_word(projected, 0x50) == 0) {
        copy_relative_payload(source, destination);
    }
}

void clone_variant_04(const void* source, void* destination) {
    std::byte projected[0xc0];
    initialize_record_and_return(projected);
    write_variant_04(source, projected);
    decode_variant_04(projected, destination);
    static_cast<std::byte*>(destination)[1] &= std::byte{0xfb};
    if (read_word(projected, 0x60) == 0) {
        copy_relative_payload(source, destination);
    }
}

void clone_variant_100(const void* source, void* destination) {
    std::byte projected[0xc0];
    initialize_record_and_return(projected);
    write_variant_100(source, projected);
    decode_variant_100(projected, destination);
    static_cast<std::byte*>(destination)[1] &= std::byte{0xfb};
    copy_relative_payload(source, destination);
}

void clone_variant_10(const void* source, void* destination) {
    std::byte projected[0xc0];
    initialize_record_and_return(projected);
    write_variant_10(source, projected);
    static_cast<std::byte*>(destination)[1] &= std::byte{0xfb};
    decode_variant_10(projected, destination);
    write_word(destination, 0x10, read_word(source, 0x10));
    write_word(destination, 0x14, read_word(source, 0x14));
    copy_relative_payload(source, destination);
}

void clone_variant_80(const void* source, void* destination) {
    std::byte projected[0xc0];
    initialize_record_and_return(projected);
    write_variant_80(source, projected);
    decode_variant_80(projected, destination);
    static_cast<std::byte*>(destination)[1] &= std::byte{0xfb};
    copy_relative_payload(source, destination);
    const std::uint32_t count = read_word(source, 0x0c);
    if (static_cast<std::int32_t>(count + 0x10u) > 0x14) {
        const std::uint32_t offset = read_word(destination, 8);
        auto* payload = offset == 0 ? nullptr :
            static_cast<std::byte*>(destination) + static_cast<std::int32_t>(offset);
        std::sprintf(reinterpret_cast<char*>(payload), "EAGL%d",
                     static_cast<std::int32_t>(read_word(destination, 0x0c)));
    }
}

void clone_variant_20(const void* source, void* destination) {
    std::byte projected[0xc0];
    initialize_record_and_return(projected);
    write_variant_20(source, projected);
    decode_variant_20(projected, destination);
    static_cast<std::byte*>(destination)[1] &= std::byte{0xfb};
    copy_relative_payload(source, destination);
}

void decode_variant_80(const void* source, void* destination) {
    initialize_record_header(destination, source);
    auto* output = static_cast<std::byte*>(destination);
    const std::uint32_t value = read_word(source, 0x84);
    output[0] = std::byte{0x69};
    set_relative_payload(destination, output + 0x10, value);
    std::sprintf(reinterpret_cast<char*>(output + 0x10), "EAGL%d",
                 static_cast<std::int32_t>(value));
}

void assemble_projection_records(void* storage, const void* source) {
    const std::uint32_t flags = read_word(source, 4);
    std::uintptr_t cursor = reinterpret_cast<std::uintptr_t>(storage);
    std::uintptr_t previous = 0;
    if (flags & 2u) {
        if (read_word(source, 0x50) == 0) {
            cursor = ((cursor + 0x9fu) & ~std::uintptr_t(0x7fu)) - 0x20u;
        }
        decode_variant_02(source, reinterpret_cast<void*>(cursor));
        previous = cursor;
        cursor += static_cast<std::uint32_t>(estimate_record_size(source));
    }
    if (flags & 4u) {
        if (read_word(source, 0x60) == 0) {
            cursor = ((cursor + 0x2fu) & ~std::uintptr_t(0x0fu)) - 0x20u;
            if (previous != 0) link_next_record(reinterpret_cast<void*>(previous),
                                                 reinterpret_cast<void*>(cursor));
        }
        decode_variant_04(source, reinterpret_cast<void*>(cursor));
        previous = cursor;
        cursor += static_cast<std::uint32_t>(estimate_variant_04_size(source));
    }
    if (flags & 0x100u) {
        cursor = (cursor + 0x2fu) & ~std::uintptr_t(0x0fu);
        const std::uintptr_t node = cursor - 0x20u;
        if (previous != 0) link_next_record(reinterpret_cast<void*>(previous),
                                             reinterpret_cast<void*>(node));
        decode_variant_100(source, reinterpret_cast<void*>(node));
        previous = node;
    }
    if (flags & 0x10u) {
        const std::uintptr_t node = ((cursor + 0x2fu) & ~std::uintptr_t(0x0fu)) - 0x20u;
        if (previous != 0) link_next_record(reinterpret_cast<void*>(previous),
                                             reinterpret_cast<void*>(node));
        decode_variant_10(source, reinterpret_cast<void*>(node));
        previous = node;
        cursor = node + static_cast<std::uint32_t>(estimate_variant_10_size(source));
    }
    if (flags & 0x80u) {
        const std::uintptr_t aligned = (cursor + 0x1fu) & ~std::uintptr_t(0x0fu);
        const std::uintptr_t node = aligned - 0x10u;
        if (previous != 0) link_next_record(reinterpret_cast<void*>(previous),
                                             reinterpret_cast<void*>(node));
        decode_variant_80(source, reinterpret_cast<void*>(node));
        previous = node;
        cursor = aligned + read_word(source, 0x84);
    }
    if (flags & 0x20u) {
        const std::uintptr_t node = ((cursor + 0x1fu) & ~std::uintptr_t(0x0fu)) - 0x10u;
        if (previous != 0) link_next_record(reinterpret_cast<void*>(previous),
                                             reinterpret_cast<void*>(node));
        decode_variant_20(source, reinterpret_cast<void*>(node));
        previous = node;
    }
    if (previous != 0) write_word(reinterpret_cast<void*>(previous), 4, 0);
}


}
