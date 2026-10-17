#include "projection/AllocationDispatch.h"
#include "projection/FormatSize.h"
#include "projection/RecordDecode.h"
#include <cstddef>
#include <cstring>

namespace hp6::projection {

void release_projection_memory(void* address) {
    DAT_00895c84->release(address, 0);
}

static std::uint32_t read_word(const void* record, std::size_t offset) {
    std::uint32_t word;
    std::memcpy(&word, static_cast<const std::byte*>(record) + offset, sizeof word);
    return word;
}

static void mark_allocated(void* record) {
    std::uint32_t word = read_word(record, 0);
    word = (word & ~0x400u) | 0x400u;
    std::memcpy(record, &word, sizeof word);
}

void* dispatch_allocation(const char* tag, std::uint32_t size,
                          std::uint32_t parameter_3, std::uint32_t parameter_4,
                          std::uint32_t parameter_5, const char* source_file,
                          std::uint32_t source_line) {
    AllocationArgument arguments[6] = {
        {6, source_line, &arguments[1]},
        {5, reinterpret_cast<std::uintptr_t>(source_file), &arguments[2]},
        {4, parameter_5, &arguments[3]},
        {3, parameter_4, &arguments[4]},
        {2, parameter_3, &arguments[5]},
        {1, reinterpret_cast<std::uintptr_t>(tag), nullptr},
    };
    return DAT_00895c84->call(size, arguments);
}

void* create_projection_collection(const void* source, std::uint32_t context) {
    auto* result = dispatch_allocation("cr_shp", estimate_collection_size(source),
                                       0x80, 0x20, context, "shpcreate.cpp", 0x311);
    assemble_projection_records(result, source);
    mark_allocated(result);
    return result;
}

void* create_clut_element(const void* source, std::uint32_t context) {
    auto* result = dispatch_allocation(
        "Created Clut Element", read_word(source, 0x84) + 0x10u,
        0x10, 0x10, context,
        "D:\\depot\\egami\\realgraph\\6\\source\\shape\\cmn\\shpcreate.cpp", 0x2c4);
    decode_variant_80(source, result);
    mark_allocated(result);
    return result;
}

void* create_shp_record_variant_20(const void* source, std::uint32_t context) {
    auto* result = dispatch_allocation("cr_cmnt", read_word(source, 0x90) + 0x11u,
                                       0x10, 0x10, context, "shpcreate.cpp", 0x2ea);
    decode_variant_20(source, result);
    mark_allocated(result);
    return result;
}

void* create_shp_record_variant_02(const void* source, std::uint32_t context) {
    auto* result = dispatch_allocation("cr_tex",
                                       static_cast<std::uint32_t>(estimate_record_size(source)),
                                       0x80, 0x20, context, "shpcreate.cpp", 0x21b);
    decode_variant_02(source, result);
    mark_allocated(result);
    return result;
}

void* create_clut_variant_04(const void* source, std::uint32_t context) {
    auto* result = dispatch_allocation(
        "Created Clut Element",
        static_cast<std::uint32_t>(estimate_variant_04_size(source)),
        0x10, 0x20, context,
        "D:\\depot\\egami\\realgraph\\6\\source\\shape\\cmn\\shpcreate.cpp", 0x246);
    decode_variant_04(source, result);
    mark_allocated(result);
    return result;
}

void* create_clip_variant_100(const void* source, std::uint32_t context) {
    auto* result = dispatch_allocation("cr_clip", 0x20, 0x10, 0x20,
                                       context, "shpcreate.cpp", 0x26f);
    decode_variant_100(source, result);
    mark_allocated(result);
    return result;
}

void* create_hots_variant_10(const void* source, std::uint32_t context) {
    auto* result = dispatch_allocation(
        "cr_hots", static_cast<std::uint32_t>(estimate_variant_10_size(source)),
        0x10, 0x20, context, "shpcreate.cpp", 0x29c);
    decode_variant_10(source, result);
    mark_allocated(result);
    return result;
}

void* clone_variant_02_allocated(const void* source, std::uint32_t context) {
    auto* result = dispatch_allocation("c_tex", read_word(source, 0x0c) + 0x20u,
                                       0x80, 0x20, context, "shpclone.cpp", 0x3c);
    clone_variant_02(source, result);
    std::uint32_t zero = 0;
    std::memcpy(static_cast<std::byte*>(result) + 4, &zero, sizeof zero);
    mark_allocated(result);
    return result;
}

void* clone_variant_04_allocated(const void* source, std::uint32_t context) {
    auto* result = dispatch_allocation("cl_clut", read_word(source, 0x0c) + 0x20u,
                                       0x10, 0x20, context, "shpclone.cpp", 0x6f);
    clone_variant_04(source, result);
    std::uint32_t zero = 0;
    std::memcpy(static_cast<std::byte*>(result) + 4, &zero, sizeof zero);
    mark_allocated(result);
    return result;
}

void* clone_variant_10_allocated(const void* source, std::uint32_t context) {
    auto* result = dispatch_allocation("cl_hots", read_word(source, 0x0c) + 0x20u,
                                       0x10, 0x20, context, "shpclone.cpp", 0xd2);
    clone_variant_10(source, result);
    std::uint32_t zero = 0;
    std::memcpy(static_cast<std::byte*>(result) + 4, &zero, sizeof zero);
    mark_allocated(result);
    return result;
}

void* clone_variant_100_allocated(const void* source, std::uint32_t context) {
    auto* result = dispatch_allocation("cl_clip", record_storage_size(source),
                                       0x10, 0x20, context, "shpclone.cpp", 0x9f);
    clone_variant_100(source, result);
    std::uint32_t zero = 0;
    std::memcpy(static_cast<std::byte*>(result) + 4, &zero, sizeof zero);
    mark_allocated(result);
    return result;
}

void* clone_variant_80_allocated(const void* source, std::uint32_t context) {
    auto* result = dispatch_allocation("cl_eagl", read_word(source, 0x0c) + 0x10u,
                                       0x10, 0x10, context, "shpclone.cpp", 0x106);
    clone_variant_80(source, result);
    std::uint32_t zero = 0;
    std::memcpy(static_cast<std::byte*>(result) + 4, &zero, sizeof zero);
    mark_allocated(result);
    return result;
}

void* clone_variant_20_allocated(const void* source, std::uint32_t context) {
    auto* result = dispatch_allocation("cl_cmnt", read_word(source, 0x0c) + 0x10u,
                                       0x10, 0x10, context, "shpclone.cpp", 0x136);
    clone_variant_20(source, result);
    std::uint32_t zero = 0;
    std::memcpy(static_cast<std::byte*>(result) + 4, &zero, sizeof zero);
    mark_allocated(result);
    return result;
}

}
