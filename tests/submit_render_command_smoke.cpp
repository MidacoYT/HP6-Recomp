// Local behavior check for hp6::render::submit_render_command (0x00542e10,
// "SubmitRenderCommand" -- 62 real non-vtable callers across the whole
// engine, the busiest single real function this project has found so
// far). Confirmed via live disassembly (the decompiled C alone
// mislabels the bucket-index parameter's real role). See
// src/render/SubmitRenderCommand.h for the full behavioural writeup and
// architecture/startup_path.md ("iMsgDoRender / GUI rendering trail")
// for how this sits in the traced BuildRenderQuad -> ... ->
// SubmitRenderCommand -> RenderAllRegions -> Scene::End chain.
#include "render/SubmitRenderCommand.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

// ---- SubmitRenderCommand.cpp's own extern dependencies ----------------
std::uint8_t* DAT_009970d8[16] = {};
std::uint8_t DAT_009978e0[256] = {};
std::uint8_t DAT_0098c690[256] = {};
char DAT_0089cbfe = 0;
std::uint8_t* DAT_00967218 = nullptr;
std::int32_t DAT_00998580 = 0;
void* DAT_0096b48c = nullptr;
void* _Dst_00b10ce8[256] = {};
void* _Dst_00966fd8[256] = {};

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}
} // namespace

int main() {
    using namespace hp6::render;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    // A real region block: >=0x10000 bytes (tails array at +0, heads
    // array at +0x8000, matching hp6::render::set_active_render_region's
    // own 0x10000-byte-stride table).
    static std::uint8_t region_block[0x10000] = {};
    DAT_00967218 = region_block;

    // The bump write-pointer starts somewhere mid-buffer so advancing it
    // is observable and distinct from "still zero".
    static std::uint8_t command_arena[0x200] = {};
    DAT_0096b48c = command_arena;

    // ---- normal mode: region descriptor's +0x14 mode bits != 0x40000000 ----
    static std::uint8_t normal_descriptor[0x30] = {};
    *reinterpret_cast<std::uint16_t*>(normal_descriptor + 0x22) = 5; // field_22
    DAT_009970d8[2] = normal_descriptor; // bucket 2 -> this descriptor
    DAT_0089cbfe = static_cast<char>(1u << 4); // the active region's own mask bit

    std::int32_t simple_command[4] = {0x11, 0x0, 0x33, 0x44}; // flags(=command[1])=0: "simple", 0x10 bytes
    void* arena_before = DAT_0096b48c;
    submit_render_command(2, simple_command);

    expect("normal_marks_region_descriptor_byte", DAT_009978e0[5] == 0x3f);
    expect("normal_marks_bucket_region_mask", (DAT_0098c690[2] & (1u << 4)) != 0);
    expect("normal_copied_simple_payload",
           std::memcmp(arena_before, simple_command, 0x10) == 0);
    expect("normal_advanced_bump_pointer_by_0x10",
           static_cast<std::uint8_t*>(DAT_0096b48c) == static_cast<std::uint8_t*>(arena_before) + 0x10);

    void** tail_slot = reinterpret_cast<void**>(DAT_00967218 + 2 * 4);
    void** head_slot = reinterpret_cast<void**>(DAT_00967218 + 0x8000 + 2 * 4);
    expect("normal_first_item_is_tail", *tail_slot == arena_before);
    expect("normal_first_item_is_also_head", *head_slot == arena_before);

    // A second, "complex" (0x24-byte) item in the same bucket: must link
    // the previous tail's own +0xc "next" field to the new item, and
    // leave the head untouched (list was not empty).
    std::int32_t complex_command[9] = {0x55, 0x40 /* a bit in 0xbfff7bcf */, 0, 0, 0, 0, 0, 0, 0};
    void* arena_before_2 = DAT_0096b48c;
    submit_render_command(2, complex_command);

    expect("normal_second_item_copied_complex_payload",
           std::memcmp(arena_before_2, complex_command, 0x24) == 0);
    expect("normal_second_item_advanced_bump_pointer_by_0x24",
           static_cast<std::uint8_t*>(DAT_0096b48c) == static_cast<std::uint8_t*>(arena_before_2) + 0x24);
    expect("normal_first_item_linked_to_second", *reinterpret_cast<void**>(
               static_cast<std::uint8_t*>(arena_before) + 0xc) == arena_before_2);
    expect("normal_tail_advanced_to_second_item", *tail_slot == arena_before_2);
    expect("normal_head_unchanged", *head_slot == arena_before);

    // ---- indexed mode: region descriptor's +0x14 mode bits == 0x40000000,
    // sub-slot 3 at bits 10-17 -------------------------------------------
    static std::uint8_t indexed_descriptor[0x30] = {};
    *reinterpret_cast<std::uint32_t*>(indexed_descriptor + 0x14) = 0x40000000u | (3u << 10);
    *reinterpret_cast<std::uint16_t*>(indexed_descriptor + 0x22) = 9; // a different field_22
    DAT_009970d8[7] = indexed_descriptor; // bucket 7 -> this descriptor
    DAT_00998580 = 0x2a; // the active render-region's own mode, copied into the item

    std::int32_t indexed_command[4] = {0x66, 0x8000 /* >>0xf == 1 */, 0x77, 0x88};
    void* arena_before_3 = DAT_0096b48c;
    submit_render_command(7, indexed_command);

    expect("indexed_marks_region_descriptor_byte", DAT_009978e0[9] == 0x3f);
    expect("indexed_first_item_is_head", _Dst_00966fd8[3] == arena_before_3);
    expect("indexed_copied_simple_payload",
           std::memcmp(arena_before_3, indexed_command, 0x10) == 0);
    auto* indexed_item = static_cast<std::uint8_t*>(arena_before_3);
    expect("indexed_appended_bucket_at_0x24",
           *reinterpret_cast<std::int16_t*>(indexed_item + 0x24) == 7);
    expect("indexed_appended_shifted_flags_at_0x26",
           *reinterpret_cast<std::int16_t*>(indexed_item + 0x26) == 1);
    expect("indexed_appended_active_region_mode_at_0x28",
           *reinterpret_cast<std::int32_t*>(indexed_item + 0x28) == 0x2a);
    expect("indexed_tail_set", _Dst_00b10ce8[3] == indexed_item);
    expect("indexed_advanced_bump_pointer_by_0x2c",
           static_cast<std::uint8_t*>(DAT_0096b48c) == indexed_item + 0x2c);

    // A second indexed-mode item in the same sub-slot: links the
    // previous tail's +0xc, leaves the (now-stale) head untouched.
    void* arena_before_4 = DAT_0096b48c;
    submit_render_command(7, indexed_command);
    expect("indexed_second_item_linked_to_first_tail",
           *reinterpret_cast<void**>(indexed_item + 0xc) == arena_before_4);
    expect("indexed_head_unchanged_on_second_item", _Dst_00966fd8[3] == arena_before_3);

    if (failures != 0) {
        std::printf("submit_render_command_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("submit_render_command_smoke: all checks passed\n");
    return 0;
}
