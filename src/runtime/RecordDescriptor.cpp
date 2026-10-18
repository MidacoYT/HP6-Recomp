#include "runtime/RecordDescriptor.h"

#include <cstring>

namespace hp6::runtime {

// ReAgent candidate: reports/pipeline/candidates/006b4678.cpp.
void clear_quad_word(std::uint32_t* block) {
    block[0] = 0;
    block[1] = 0;
    block[2] = 0;
    block[3] = 0;
}

// ReAgent candidate: reports/pipeline/rejected/006b6808.cpp. Rejected
// only for raw offsets (unresolved record layout, house style).
void clear_full_record(std::uint32_t* block) {
    for (int i = 0; i < 0x10; ++i) {
        block[i] = 0;
    }
    block[0x10] = 0;
    block[0x11] = 0;
    block[0x12] = 0;
    block[0x13] = 0;
    clear_quad_word(block + 0x14);
    *(reinterpret_cast<std::uint8_t*>(block) + 0x60) = 0;
}

// ReAgent candidate build/validation failed outright; reconstructed
// directly from the exported x86 (matches clear_full_record's layout).
void construct_full_record(void* record, const char* name, std::uint32_t field_0x40,
                           std::uint32_t field_0x44, std::uint32_t field_0x48,
                           std::uint32_t field_0x4c, const std::uint32_t* optional_id) {
    auto* words = static_cast<std::uint32_t*>(record);
    clear_full_record(words);
    std::strncpy(static_cast<char*>(record), name, 0x40);
    words[0x10] = field_0x40;
    words[0x11] = field_0x44;
    words[0x12] = field_0x48;
    words[0x13] = field_0x4c;
    if (optional_id != nullptr) {
        words[0x14] = optional_id[0];
        words[0x15] = optional_id[1];
        words[0x16] = optional_id[2];
        words[0x17] = optional_id[3];
    }
    *(reinterpret_cast<std::uint8_t*>(record) + 0x60) = 0;
}

// ReAgent candidate: reports/pipeline/rejected/006b687f.cpp. Rejected
// only for raw offsets (same house style as clear_full_record).
void clear_named_record(std::uint32_t* block) {
    for (int i = 0; i < 0x10; ++i) {
        block[i] = 0;
    }
    clear_quad_word(block + 0x10);
}

// ReAgent candidate: reports/pipeline/rejected/006b6894.cpp. Rejected
// only for raw offsets.
void construct_named_record(void* record, const char* name,
                            const std::uint32_t* optional_id) {
    auto* words = static_cast<std::uint32_t*>(record);
    clear_named_record(words);
    std::strncpy(static_cast<char*>(record), name, 0x40);
    if (optional_id != nullptr) {
        words[0x10] = optional_id[0];
        words[0x11] = optional_id[1];
        words[0x12] = optional_id[2];
        words[0x13] = optional_id[3];
    }
}

// ReAgent candidate: reports/pipeline/rejected/006b4c17.cpp. Rejected
// only for raw offsets (unresolved owning record).
void initialize_object_name_header(void* object, const char* name) {
    auto* bytes = static_cast<std::uint8_t*>(object);
    std::memset(bytes + 0xb4, 0, 0x20);
    std::strncpy(reinterpret_cast<char*>(bytes + 0xb4), name, 0x1f);
    *reinterpret_cast<std::uint32_t*>(bytes + 0x4dc) = 0xffffffffu;
    *reinterpret_cast<std::uint32_t*>(bytes + 0x4e4) = 0;
    *reinterpret_cast<std::uint32_t*>(bytes + 0x4d8) = 4;
    *reinterpret_cast<std::uint32_t*>(bytes + 0x6c) = 4;
    *reinterpret_cast<std::uint32_t*>(bytes + 0x70) = 9;
}

} // namespace hp6::runtime
