#include "projection/RecordDecode.h"
#include "projection/RecordFields.h"
#include "projection/RecordFlags.h"
#include "projection/FormatSize.h"
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <cstdio>

namespace hp6::projection {

// Global externe DAT_00895c84 : pointeur vers une table de pointeurs de fonction (vtable).
// Le premier element est invoque par invoke_global_callback.
extern "C" void** g_callback_vtable_00895c84;
void** g_callback_vtable_00895c84 = nullptr; // DAT_00895c84 : non résolu, pointeur externe

using CallbackFn = void (*)(void*, std::uint32_t*);


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
        const std::uint32_t source_offset = read_word(source, 0x08);
        const std::uint32_t destination_offset = read_word(destination, 0x08);
        const auto* input_data = source_offset == 0 ? nullptr :
            static_cast<const std::byte*>(source) + static_cast<std::int32_t>(source_offset);
        auto* output_data = destination_offset == 0 ? nullptr :
            output + static_cast<std::int32_t>(destination_offset);
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

void invoke_global_callback(void* param1, void* param2, void* param3, void* param4,
                            void* param5, void* param6, void* param7) {
    // DAT_006d6ab6 : prepare des valeurs 1..6 sur la pile (stores morts observes),
    // puis invoque le rappel via (*DAT_00895c84)(param_2, &compteur_6).
    // Le rappel est censé retourner un pointeur (utilisé par les appelants comme this).
    std::uint32_t local_10 = 1u;
    std::uint32_t local_1c = 2u;
    std::uint32_t local_28 = 3u;
    std::uint32_t local_34 = 4u;
    std::uint32_t local_40 = 5u;
    std::uint32_t local_4c = 6u;
    std::uint32_t* ptrs[] = {&local_10, &local_1c, &local_28, &local_34, &local_40, &local_4c};
    (void)ptrs; // prepares observes, non lues avant l'appel dans ce cadre
    std::uintptr_t locals[24] = {};
    std::memcpy(&locals[0], &param1, sizeof param1);
    std::memcpy(&locals[4], &param3, sizeof param3);
    std::memcpy(&locals[8], &param4, sizeof param4);
    std::memcpy(&locals[12], &param5, sizeof param5);
    std::memcpy(&locals[16], &param6, sizeof param6);
    std::memcpy(&locals[20], &param7, sizeof param7);
    (void)locals;
    std::uint32_t count = 6u;
    auto* callback = reinterpret_cast<CallbackFn>(*g_callback_vtable_00895c84);
    callback(param2, &count);
    return;
}

void construct_variant_80_record(void* this_, int source) {
    initialize_record_header(this_, reinterpret_cast<const void*>(source));
    const std::uint32_t size = read_word(reinterpret_cast<const void*>(source), 0x84);
    static_cast<std::byte*>(this_)[0] = std::byte{0x69};
    set_relative_payload(this_, static_cast<std::byte*>(this_) + 0x10, size);
    std::uint32_t relative_offset = read_word(this_, 0x08);
    char* payload_addr = nullptr;
    if (relative_offset != 0u) {
        payload_addr = reinterpret_cast<char*>(
            reinterpret_cast<std::byte*>(this_) + static_cast<std::int32_t>(relative_offset));
    }
    // sprintf(payload_addr, "EAGL%d", size) — adresse globale 0x0071b12c = "EAGL%d"
    if (payload_addr != nullptr) {
        std::sprintf(payload_addr, "EAGL%d", size);
    }
}

std::uint32_t* create_clut_element(int param_1, std::uint32_t param_2) {
    // 0x0071b0b8 = "Created Clut Element", 0x0071b0d0 = chemin source
    static const char* clut_name = "Created Clut Element";
    static const char* source_path = "D:\\depot\\egami\\realgraph\\6\\source\\shape\\cmn\\shpcreate.cpp";
    std::uint32_t size_arg = read_word(reinterpret_cast<const void*>(param_1), 0x84) + 0x10u;
    invoke_global_callback(reinterpret_cast<void*>(const_cast<char*>(clut_name)),
                           reinterpret_cast<void*>(size_arg),
                           reinterpret_cast<void*>(0x10u),
                           reinterpret_cast<void*>(0x10u),
                           reinterpret_cast<void*>(param_2),
                           reinterpret_cast<void*>(const_cast<char*>(source_path)),
                           reinterpret_cast<void*>(0x2c4u));
    // Le callback global est cense retourner un pointeur vers un enregistrement alloue.
    // Comme son comportement est externe (DAT_00895c84), nous ne pouvons pas le connaitre.
    // Cette fonction retourne nullptr pour refletera l'incertitude ; l'appelant doit verifier.
    return nullptr;
}

std::uint32_t* create_shp_record_variant_20(int param_1, std::uint32_t param_2) {
    // 0x0071b134 = chemin source inconnu (adresse conservee)
    std::uint32_t size_arg =
        read_word(reinterpret_cast<const void*>(param_1), 0x90) + 0x11u;
    invoke_global_callback(nullptr,
                           reinterpret_cast<void*>(size_arg),
                           reinterpret_cast<void*>(0x10u),
                           reinterpret_cast<void*>(0x10u),
                           reinterpret_cast<void*>(param_2),
                           reinterpret_cast<void*>(const_cast<char*>("shpcreate.cpp")),
                           reinterpret_cast<void*>(0x2eau));
    return nullptr;
}

std::uint32_t* create_shp_record_variant_02(int param_1, std::uint32_t param_2) {
    // 0x0071b13c = chemin source inconnu (adresse conservee)
    const std::uint32_t estimated_size =
        estimate_record_size(reinterpret_cast<const void*>(param_1));
    invoke_global_callback(nullptr,
                           reinterpret_cast<void*>(estimated_size),
                           reinterpret_cast<void*>(0x80u),
                           reinterpret_cast<void*>(0x20u),
                           reinterpret_cast<void*>(param_2),
                           reinterpret_cast<void*>(const_cast<char*>("shpcreate.cpp")),
                           reinterpret_cast<void*>(0x21bu));
    return nullptr;
}


}
