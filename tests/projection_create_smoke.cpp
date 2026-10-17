#include "projection/AllocationDispatch.h"
#include "projection/FormatSize.h"
#include "projection/RecordDecode.h"
#include "projection/ChainClone.h"
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace hp6::projection {
AllocationDispatch* DAT_00895c84 = nullptr;
}

struct TestDispatch final : hp6::projection::AllocationDispatch {
    alignas(128) std::byte storage[256] = {};
    std::uint32_t requested_size = 0;
    bool arguments_valid = false;
    const char* expected_tag = "cr_shp";
    const char* expected_file = "shpcreate.cpp";
    std::uint32_t expected_line = 0x311;
    std::uint32_t expected_parameter_3 = 0x80;
    std::uint32_t expected_parameter_4 = 0x20;
    std::uint32_t expected_size = 0x20;

    void* call(std::uint32_t size, hp6::projection::AllocationArgument* args) override {
        requested_size = size;
        arguments_valid = size == expected_size &&
            args[0].index == 6 && args[0].value == expected_line &&
            args[0].next == &args[1] && args[1].index == 5 &&
            std::strcmp(reinterpret_cast<const char*>(args[1].value), expected_file) == 0 &&
            args[2].index == 4 && args[2].value == 77 &&
            args[3].index == 3 && args[3].value == expected_parameter_4 &&
            args[4].index == 2 && args[4].value == expected_parameter_3 &&
            args[5].index == 1 && args[5].next == nullptr &&
            std::strcmp(reinterpret_cast<const char*>(args[5].value), expected_tag) == 0;
        return storage;
    }
    void release(void*, std::uint32_t) override {}
};

int main() {
    std::byte source[0xc0] = {};
    auto set_word = [](std::byte* bytes, std::size_t offset, std::uint32_t value) {
        std::memcpy(bytes + offset, &value, sizeof value);
    };
    auto get_word = [](const std::byte* bytes, std::size_t offset) {
        std::uint32_t value;
        std::memcpy(&value, bytes + offset, sizeof value);
        return value;
    };
    set_word(source, 4, 0x100);
    set_word(source, 0x64, 11);
    set_word(source, 0x68, 22);
    set_word(source, 0x6c, 33);
    set_word(source, 0x70, 44);
    if (hp6::projection::estimate_collection_size(source) != 0x20) return 1;
    TestDispatch dispatch;
    hp6::projection::DAT_00895c84 = &dispatch;
    void* result = hp6::projection::create_projection_collection(source, 77);
    if (result != dispatch.storage || dispatch.requested_size != 0x20 ||
        !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0xffu) != 0x7a ||
        (get_word(dispatch.storage, 0) & 0x400u) == 0 ||
        get_word(dispatch.storage, 4) != 0 ||
        get_word(dispatch.storage, 0x10) != 11 ||
        get_word(dispatch.storage, 0x1c) != 44) return 2;
    std::byte variant80[0x40] = {};
    set_word(source, 0x84, 16);
    hp6::projection::decode_variant_80(source, variant80);
    if ((get_word(variant80, 0) & 0xffu) != 0x69 ||
        get_word(variant80, 8) != 0x10 ||
        get_word(variant80, 0x0c) != 16 ||
        std::strcmp(reinterpret_cast<const char*>(variant80 + 0x10), "EAGL16") != 0)
        return 3;
    set_word(source, 4, 0x80);
    if (hp6::projection::estimate_collection_size(source) != 0x20) return 4;
    set_word(source, 4, 0x20);
    set_word(source, 0x90, 3);
    if (hp6::projection::estimate_collection_size(source) != 0x14) return 5;
    dispatch.expected_tag = "Created Clut Element";
    dispatch.expected_file = "D:\\depot\\egami\\realgraph\\6\\source\\shape\\cmn\\shpcreate.cpp";
    dispatch.expected_line = 0x2c4;
    dispatch.expected_parameter_3 = 0x10;
    dispatch.expected_parameter_4 = 0x10;
    dispatch.expected_size = 0x20;
    result = hp6::projection::create_clut_element(source, 77);
    if (result != dispatch.storage || !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0xffu) != 0x69 ||
        (get_word(dispatch.storage, 0) & 0x400u) == 0) return 6;
    dispatch.expected_tag = "cr_cmnt";
    dispatch.expected_file = "shpcreate.cpp";
    dispatch.expected_line = 0x2ea;
    dispatch.expected_size = 0x14;
    result = hp6::projection::create_shp_record_variant_20(source, 77);
    if (result != dispatch.storage || !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0xffu) != 0x6f ||
        (get_word(dispatch.storage, 0) & 0x400u) == 0) return 7;
    dispatch.expected_tag = "cr_tex";
    dispatch.expected_line = 0x21b;
    dispatch.expected_parameter_3 = 0x80;
    dispatch.expected_parameter_4 = 0x20;
    dispatch.expected_size = 36;
    set_word(source, 0x14, 1);
    set_word(source, 0x18, 4);
    set_word(source, 0x1c, 2);
    set_word(source, 0x28, 0);
    result = hp6::projection::create_shp_record_variant_02(source, 77);
    if (result != dispatch.storage || !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0xffu) != 1 ||
        (get_word(dispatch.storage, 0) & 0x400u) == 0) return 8;
    dispatch.expected_tag = "Created Clut Element";
    dispatch.expected_file = "D:\\depot\\egami\\realgraph\\6\\source\\shape\\cmn\\shpcreate.cpp";
    dispatch.expected_line = 0x246;
    dispatch.expected_parameter_3 = 0x10;
    dispatch.expected_size = 42;
    set_word(source, 0x54, 2);
    set_word(source, 0x5c, 10);
    set_word(source, 0x60, 0);
    result = hp6::projection::create_clut_variant_04(source, 77);
    if (result != dispatch.storage || !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0xffu) != 2 ||
        (get_word(dispatch.storage, 0) & 0x400u) == 0) return 9;
    dispatch.expected_tag = "cr_clip";
    dispatch.expected_file = "shpcreate.cpp";
    dispatch.expected_line = 0x26f;
    dispatch.expected_size = 0x20;
    result = hp6::projection::create_clip_variant_100(source, 77);
    if (result != dispatch.storage || !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0xffu) != 0x7a) return 10;
    dispatch.expected_tag = "cr_hots";
    dispatch.expected_line = 0x29c;
    dispatch.expected_size = 80;
    set_word(source, 0x7c, 3);
    set_word(source, 0x80, 4);
    result = hp6::projection::create_hots_variant_10(source, 77);
    if (result != dispatch.storage || !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0xffu) != 0x7c) return 11;
    std::byte clone_source[0x60] = {};
    set_word(clone_source, 0, 1);
    set_word(clone_source, 0x08, 0x20);
    set_word(clone_source, 0x0c, 4);
    set_word(clone_source, 0x18, 4);
    set_word(clone_source, 0x1c, 2);
    std::memcpy(clone_source + 0x20, "ABCD", 4);
    dispatch.expected_tag = "c_tex";
    dispatch.expected_file = "shpclone.cpp";
    dispatch.expected_line = 0x3c;
    dispatch.expected_parameter_3 = 0x80;
    dispatch.expected_size = 36;
    result = hp6::projection::clone_variant_02_allocated(clone_source, 77);
    if (result != dispatch.storage || !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0xffu) != 1 ||
        get_word(dispatch.storage, 4) != 0 ||
        std::memcmp(dispatch.storage + 0x20, "ABCD", 4) != 0) return 12;
    std::memset(clone_source, 0, sizeof clone_source);
    set_word(clone_source, 0, 2);
    set_word(clone_source, 0x08, 0x20);
    set_word(clone_source, 0x0c, 10);
    set_word(clone_source, 0x18, 10);
    std::memcpy(clone_source + 0x20, "ABCDEFGHIJ", 10);
    dispatch.expected_tag = "cl_clut";
    dispatch.expected_line = 0x6f;
    dispatch.expected_parameter_3 = 0x10;
    dispatch.expected_size = 42;
    result = hp6::projection::clone_variant_04_allocated(clone_source, 77);
    if (result != dispatch.storage || !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0xffu) != 2 ||
        get_word(dispatch.storage, 4) != 0 ||
        std::memcmp(dispatch.storage + 0x20, "ABCDEFGHIJ", 10) != 0) return 13;
    std::memset(clone_source, 0, sizeof clone_source);
    set_word(clone_source, 0, 0x7c);
    set_word(clone_source, 0x08, 0x20);
    set_word(clone_source, 0x0c, 48);
    set_word(clone_source, 0x10, 7);
    set_word(clone_source, 0x14, 8);
    set_word(clone_source, 0x18, 3);
    set_word(clone_source, 0x1c, 4);
    std::memset(clone_source + 0x20, 0x5a, 48);
    dispatch.expected_tag = "cl_hots";
    dispatch.expected_line = 0xd2;
    dispatch.expected_size = 80;
    result = hp6::projection::clone_variant_10_allocated(clone_source, 77);
    if (result != dispatch.storage || !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0xffu) != 0x7c ||
        get_word(dispatch.storage, 4) != 0 ||
        std::memcmp(dispatch.storage + 0x20, clone_source + 0x20, 48) != 0)
        return 14;
    std::memset(clone_source, 0, sizeof clone_source);
    set_word(clone_source, 0, 0x7a);
    set_word(clone_source, 0x08, 0x20);
    set_word(clone_source, 0x0c, 4);
    std::memcpy(clone_source + 0x20, "WXYZ", 4);
    dispatch.expected_tag = "cl_clip";
    dispatch.expected_line = 0x9f;
    dispatch.expected_size = 36;
    result = hp6::projection::clone_variant_100_allocated(clone_source, 77);
    if (result != dispatch.storage || !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0xffu) != 0x7a ||
        get_word(dispatch.storage, 4) != 0 ||
        std::memcmp(dispatch.storage + 0x20, "WXYZ", 4) != 0) return 15;
    std::memset(clone_source, 0, sizeof clone_source);
    set_word(clone_source, 0, 0x69);
    set_word(clone_source, 0x08, 0x10);
    set_word(clone_source, 0x0c, 16);
    std::memset(clone_source + 0x10, 'X', 16);
    dispatch.expected_tag = "cl_eagl";
    dispatch.expected_line = 0x106;
    dispatch.expected_parameter_4 = 0x10;
    dispatch.expected_size = 32;
    result = hp6::projection::clone_variant_80_allocated(clone_source, 77);
    if (result != dispatch.storage || !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0xffu) != 0x69 ||
        get_word(dispatch.storage, 4) != 0 ||
        std::strcmp(reinterpret_cast<const char*>(dispatch.storage + 0x10), "EAGL16") != 0)
        return 16;
    std::memset(clone_source, 0, sizeof clone_source);
    set_word(clone_source, 0, 0x6f);
    set_word(clone_source, 0x08, 0x10);
    set_word(clone_source, 0x0c, 4);
    std::memcpy(clone_source + 0x10, "abc", 4);
    dispatch.expected_tag = "cl_cmnt";
    dispatch.expected_line = 0x136;
    dispatch.expected_size = 20;
    result = hp6::projection::clone_variant_20_allocated(clone_source, 77);
    if (result != dispatch.storage || !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0xffu) != 0x6f ||
        get_word(dispatch.storage, 4) != 0 ||
        std::strcmp(reinterpret_cast<const char*>(dispatch.storage + 0x10), "abc") != 0)
        return 17;
    std::memset(clone_source, 0, sizeof clone_source);
    set_word(clone_source, 0, 0x6f);
    set_word(clone_source, 4, 0x20);
    set_word(clone_source, 8, 0x10);
    set_word(clone_source, 0x0c, 4);
    std::memcpy(clone_source + 0x10, "one", 4);
    set_word(clone_source, 0x20, 0x6f);
    set_word(clone_source, 0x28, 0x10);
    set_word(clone_source, 0x2c, 4);
    std::memcpy(clone_source + 0x30, "two", 4);
    if (hp6::projection::measure_clone_chain(clone_source) != 52) return 18;
    std::memset(dispatch.storage, 0, sizeof dispatch.storage);
    dispatch.expected_tag = "cl_shp";
    dispatch.expected_line = 0x1c4;
    dispatch.expected_parameter_3 = 0x80;
    dispatch.expected_parameter_4 = 0x20;
    dispatch.expected_size = 52;
    result = hp6::projection::clone_record_chain_allocated(clone_source, 77);
    if (result != dispatch.storage || !dispatch.arguments_valid ||
        (get_word(dispatch.storage, 0) & 0x400u) == 0 ||
        std::strcmp(reinterpret_cast<const char*>(dispatch.storage + 0x10), "one") != 0 ||
        std::strcmp(reinterpret_cast<const char*>(dispatch.storage + 0x30), "two") != 0 ||
        get_word(dispatch.storage, 4) != 0x20 ||
        get_word(dispatch.storage, 0x24) != 0) return 19;
    return 0;
}
