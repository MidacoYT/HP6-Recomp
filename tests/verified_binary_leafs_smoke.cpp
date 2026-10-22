#include "runtime/VerifiedBinaryLeafs.h"
#include "platform/Sync.h"
#include "audio/PathVolume.h"

#include <cstdint>
#include <cstring>

std::int32_t DAT_00998580 = 0;

int main() {
    using namespace hp6::binary_leafs;
    if (fn_00401220(0x89abcdefu) != 0x89abcdefu) return 1;
    DAT_00998580 = -7;
    if (fn_00404de0() != static_cast<std::uint32_t>(-7)) return 2;
    if (fn_00407b30() != 1 || fn_004d59c0() != 2 ||
        fn_004eef20() != 3 || fn_00622a70() != 4) return 3;
    if (fn_006de5de() != 0 || fn_006e6016() != 0) return 4;

    alignas(4) std::uint8_t record[0x28] = {};
    std::uint32_t word = 0x12345678u;
    std::memcpy(record + 8, &word, sizeof word);
    if (fn_0062b2c0(record) != word) return 5;
    fn_0050c0c0(record);
    if (fn_0062b2c0(record) != 0) return 6;
    record[4] = 0xa5;
    if (fn_00653600(record) != 0xa5) return 7;
    std::memcpy(record + 0x20, &word, sizeof word);
    if (hp6::platform::lock_acquisition_depth(record) != word ||
        fn_006a7e50(&word) != word) return 8;

    alignas(4) std::uint8_t large_record[0x248] = {};
    const auto put_word = [&](std::size_t offset, std::uint32_t value) {
        std::memcpy(large_record + offset, &value, sizeof value);
    };
    put_word(0x04, 4);
    put_word(0x10, 16);
    put_word(0x14, 20);
    put_word(0x24, 36);
    put_word(0x58, 88);
    put_word(0x64, 100);
    if (fn_006bdfa0(large_record) != 4 ||
        hp6::audio::path_get_dry_level(large_record) != 16 ||
        fn_006bdf50(large_record) != 20 ||
        hp6::audio::path_get_volume(large_record) != 20 ||
        fn_006bdfc0(large_record) != 36 || fn_006c1d00(large_record) != 88 ||
        fn_006bde50(large_record) != 100) return 9;
    large_record[0x1ac] = 0xd3;
    put_word(0x220, 7);
    put_word(0x224, 11);
    put_word(0x244, 0xfffffffdu);
    if (fn_006cec20(large_record) != 0xd3 ||
        fn_006c9e80(large_record) != 0xfffffffdu ||
        fn_006c9e90(large_record) != 4 || fn_006c9eb0(large_record) != 8) return 10;

    std::uint32_t pair[2] = {17, 19};
    fn_006b477f(pair);
    if (pair[0] != 0 || pair[1] != 0) return 11;
    std::uint32_t triple[3] = {23, 29, 31};
    fn_006e712e(triple);
    if (triple[0] != 0 || triple[1] != 0 || triple[2] != 0) return 12;

    fn_00405a30();
    fn_006ae620();
    fn_006c9700();
    fn_006dadcf();
    fn_006e208d();
    return 0;
}
