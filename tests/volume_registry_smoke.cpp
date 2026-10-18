// Local behavior checks for batch volume_registry_044: the CRT memset body
// wrapper (006b3b0e) and the volume lookup by path prefix (006b25b8). CRT
// imports 006b3c8c/006b92bc/006b9392/006b93a4 are exercised here through
// their standard functions. Game-space symbols get test-local definitions.
#include "runtime/VolumeRegistry.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#include <windows.h>

std::int32_t DAT_00892b60 = 0;

namespace {

int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}

std::uint32_t address(const void* value) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}

std::uint32_t& word(void* bytes, std::size_t offset) {
    return *reinterpret_cast<std::uint32_t*>(static_cast<std::uint8_t*>(bytes) + offset);
}

// Volume object: the name getter sits at vtable slot 0x30/4 (index 12).
struct VolumeNode {
    virtual void v0() {}
    virtual void v1() {}
    virtual void v2() {}
    virtual void v3() {}
    virtual void v4() {}
    virtual void v5() {}
    virtual void v6() {}
    virtual void v7() {}
    virtual void v8() {}
    virtual void v9() {}
    virtual void va() {}
    // vb() sits at vtable slot +0x2c (index 11), the slot
    // invoke_volume_method_2c (006b2673) tail-jumps into.
    virtual void vb() { ++method_2c_calls; }
    const char* name_text;
    int method_2c_calls = 0;
    explicit VolumeNode(const char* text) : name_text(text) {}
    virtual char* name(char* /*buffer*/) {
        // The +0x30 slot answers with the record's own name string; it
        // must not touch the caller's buffer (otherwise the following
        // _stricmp would compare the record name against itself and the
        // scan would stop at the first record).
        return const_cast<char*>(name_text);
    }
};

alignas(16) std::uint8_t root[0x100];
alignas(16) char* game_path_slot[1];

} // namespace

int main() {
    using namespace hp6::runtime;
    static_assert(sizeof(void*) == 4, "The binary stores 32-bit addresses");
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // ---- clear_buffer (006b3b0e) ---------------------------------------
    std::printf("[m1] clear_buffer\n");
    alignas(16) std::uint8_t scratch[24];
    std::memset(scratch, 0xcd, sizeof scratch);
    clear_buffer(reinterpret_cast<std::uintptr_t>(scratch) + 4, 12);
    expect("clear_window", scratch[3] == 0xcd && scratch[4] == 0 && scratch[15] == 0 &&
                               scratch[16] == 0xcd);

    // ---- find_volume_by_name (006b25b8) ---------------------------------
    std::memset(root, 0, sizeof root);
    DAT_00892b60 = static_cast<std::int32_t>(address(root));
    word(root, 0x98) = address(scratch);            // early return value

    std::printf("[m2] base return\n");
    // No colon, no path separator: the engine slot base comes back.
    expect("volume_base", find_volume_by_name("level02") ==
                              scratch);

    VolumeNode volume_d("D:");
    VolumeNode volume_c("C:");
    alignas(16) std::uint8_t record_d[0x88];
    alignas(16) std::uint8_t record_c[0x88];
    std::memset(record_d, 0, sizeof record_d);
    std::memset(record_c, 0, sizeof record_c);
    word(record_d, 0x84) = address(&volume_d);
    word(record_c, 0x84) = address(&volume_c);
    word(record_d, 0) = address(record_c);          // chain d -> c
    word(root, 0x7c) = address(record_d);

    // Two elements compared with _stricmp; the second matches.
    std::printf("[m3] chain\n");
    word(root, 0x7c) = address(record_d);
    expect("volume_chain", find_volume_by_name("c:\\levels") == record_c);
    expect("volume_case", find_volume_by_name("D:X") == record_d);

    // Slash path without colon: prefix taken from the game path double
    // deref at root + 0x88.
    std::printf("[m4] game path\n");
    static char game_path[] = "D:\\base\\game.big";
    game_path_slot[0] = game_path;
    word(root, 0x88) = address(game_path_slot);
    expect("volume_game_path", find_volume_by_name("\\data") == record_d);
    std::printf("[m5] missing\n");

    // Nothing matches: null.
    word(root, 0x7c) = address(record_d);
    expect("volume_missing", find_volume_by_name("Y:\\x") == nullptr);

    // ---- invoke_volume_method_2c (006b2673) -----------------------------
    std::printf("[m6] method 2c\n");
    word(root, 0x7c) = address(record_d);
    invoke_volume_method_2c("D:X");
    expect("method_2c_called", volume_d.method_2c_calls == 1);
    expect("method_2c_other_untouched", volume_c.method_2c_calls == 0);
    invoke_volume_method_2c("c:\\levels");
    expect("method_2c_second_record", volume_c.method_2c_calls == 1 &&
                                      volume_d.method_2c_calls == 1);

    if (failures != 0) {
        std::printf("volume_registry_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("volume_registry_smoke: all checks passed\n");
    return 0;
}
