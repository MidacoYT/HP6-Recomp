// Smoke test for hp6::presentation::TextLayout (the 0x005e92d0..0x005e9820
// chain) and hp6::render::GeometryBatcher (0x01003b20/0x01003ce0/0x006516f0).
// Every game global referenced below is DEFINED here for the test link; the
// stub behaviours are only the contracts the reconstructions already decode
// (allocator slot 0 / release slot +0xc of the engine service holder tables,
// the DAT_00cf18e4 bump pool, and the submit_render_command queues).
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "math/Transform.h"
#include "presentation/BitmapRenderQuad.h"
#include "presentation/GuiTextTable.h"
#include "presentation/TextLayout.h"
#include "render/GeometryBatcher.h"

// ---- test-side definitions of the game address-space symbols --------------
std::int32_t* DAT_00cf18e4 = nullptr;          // bump arena
volatile std::uint32_t _DAT_00000000 = 0;      // null-store trap target
void* DAT_0096b48c = nullptr;                  // bump write-pointer
void* DAT_00fcdd74 = nullptr;                  // service holder slot 0
void* DAT_00fcdd78 = nullptr;                  // service holder slot 1
void** DAT_00cef6bc = nullptr;                 // watch pointer
std::uint32_t DAT_00fd1254 = 0;                // lazy-init flag
void* PTR_FUN_007159b0 = nullptr;
void* PTR_FUN_00715d0c = nullptr;
void* PTR_LAB_00715da8 = nullptr;
void* PTR_LAB_0071b520 = nullptr;
std::uint32_t DAT_008998c8 = 0;
void* DAT_00fcaec0 = nullptr;                  // pointer-valued global
std::uint16_t DAT_00d714b8 = 0x0099;
std::uint16_t DAT_00b1089c = 0x0bad;
std::uint32_t DAT_00728004 = 0;                // plain-glyph corner constant
std::uint32_t DAT_0082ae34 = 0;
// Index pattern table: the PE dword at 0x008928d8 = 0x00010002 and the
// PE dword at 0x008928dc = 0x00020001 (each dword carries one word pair).
std::uint32_t DAT_008928d8 = 0x00010002u;
std::uint32_t DAT_008928dc = 0x00020001u;
float DAT_007db234 = 1.0f;
std::uint8_t DAT_008663d5 = 0;
void* DAT_00cf19f0 = nullptr;
void* DAT_00cf19f4 = nullptr;
void* DAT_00cf19f8 = nullptr;
void* DAT_00cf19fc = nullptr;
void** DAT_00cf1a00 = nullptr;
void* _DAT_00cf1a04 = nullptr;
std::uint32_t* DAT_00cf1a08 = nullptr;
void* _DAT_00cf1a0c = nullptr;
std::uint32_t DAT_00cf1a10 = 0;
std::uint32_t DAT_00cf1a14 = 0;
std::uint32_t DAT_00cf1a18 = 0;
std::uint32_t DAT_00cf1a1c = 0;
void* DAT_00cf1a20 = nullptr;
std::uint32_t DAT_00cf1a24 = 0;
std::uint32_t DAT_00cf19de = 0;
void* DAT_00cf1944 = nullptr;
std::uint32_t DAT_008663e4 = 0;
std::uint32_t DAT_008663e8 = 0;
std::uint32_t DAT_008663ec = 0;
std::uint32_t DAT_00cf1a28 = 0;
std::uint32_t DAT_00cf1a2c = 0;
std::uint32_t DAT_008663f0[16] = {};
std::uint8_t DAT_0098b490[0x48] = {};
std::uint8_t DAT_00bb09f8[0x48] = {};
std::uint8_t DAT_00a70890[0x14 * 32] = {};
std::uint8_t DAT_00baa170[0x14 * 32] = {};
std::uint8_t DAT_0098c6a8[0x14 * 32] = {};
std::uint8_t DAT_009985a0[0x14 * 32] = {};
std::uint8_t* DAT_009970d8[0x2000] = {};
std::uint8_t DAT_009978e0[256] = {};
std::uint8_t DAT_0098c690[0x2000] = {};
char DAT_0089cbfe = 0;
std::uint8_t* DAT_00967218 = nullptr;
std::int32_t DAT_00998580 = 0;
void* _Dst_00b10ce8[256] = {};
void* _Dst_00966fd8[256] = {};
void cleanup_00711e60() {}

// D3D9 device and buffer vtables for the pool/Lock path. They reproduce
// only the methods 00651d60/00651f80/00650f70/006512f0 actually call.
struct FakeBuffer { void** vtable; std::uint8_t* data; };
struct FakeDevice { void** vtable; };
static void* fake_buffer_vtable[13] = {};
static void* fake_device_vtable[28] = {};
static FakeDevice fake_device{fake_device_vtable};
static FakeBuffer fake_buffers[32] = {};
static std::uint8_t fake_buffer_data[32][0x20000] = {};
static int fake_buffer_count = 0;
static int fake_lock_count = 0;
static int fake_unlock_count = 0;
static long __stdcall fake_create_vertex(void*, unsigned, unsigned, unsigned,
                                         unsigned, void** output, void*) {
    if (fake_buffer_count >= 32) {
        std::printf("fake D3D buffer fixture exhausted: %d\n", fake_buffer_count);
        std::abort();
    }
    auto& buffer = fake_buffers[fake_buffer_count];
    buffer.vtable = fake_buffer_vtable;
    buffer.data = fake_buffer_data[fake_buffer_count++];
    *output = &buffer;
    return 0;
}
static long __stdcall fake_create_index(void* device, unsigned size,
                                        unsigned usage, unsigned format,
                                        unsigned pool, void** output, void* handle) {
    return fake_create_vertex(device, size, usage, format, pool, output, handle);
}
static long __stdcall fake_lock(void* object, unsigned offset, unsigned,
                                void** output, unsigned) {
    ++fake_lock_count;
    *output = static_cast<FakeBuffer*>(object)->data + offset;
    return 0;
}
static long __stdcall fake_unlock(void*) {
    ++fake_unlock_count;
    return 0;
}

int failures = 0;
static void expect(const char* name, bool condition) {
    std::printf("%-46s %s\n", name, condition ? "ok" : "FAIL");
    if (!condition) {
        ++failures;
    }
}

static std::uint32_t address_of(const void* value) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}

static std::uint32_t bits_of(float value) {
    std::uint32_t bits;
    std::memcpy(&bits, &value, 4);
    return bits;
}

// ---- eastl allocator / service-holder stubs -------------------------------
static int stub_error_context_matches = 0;
static alignas(16) std::uint8_t eastl_pool[0x10000];
static std::uint8_t* eastl_cursor = eastl_pool;
static void* test_eastl_allocate(std::uint32_t size, void* context) {
    // 005e27e0's context frame: {1, "eastl_overflow_error_allocator", 0}.
    struct AllocatorContext {
        std::int32_t level;
        const char* name;
        void* previous;
    };
    const auto* frame = static_cast<const AllocatorContext*>(context);
    if (frame->level == 1 &&
        std::strcmp(frame->name, "eastl_overflow_error_allocator") == 0 &&
        frame->previous == nullptr) {
        ++stub_error_context_matches;
    }
    void* block = eastl_cursor;
    eastl_cursor += size;
    return block;
}
static bool stub_freed_stack_scratch = false;
static void service_release_stub(void* /*holder*/, void* object,
                                 std::uint32_t /*mode*/) {
    if (object == static_cast<void*>(&stub_freed_stack_scratch)) {
        stub_freed_stack_scratch = true; // never arranged to happen below
    }
}
// Slot +0xc of table 0 is read at address(PTR_FUN_0083e55c) + 0xc; this
// test never dispatches it (no eastl block is freed), so a scalar is
// enough — its only consumer is service_holder's one-shot init.
void* PTR_FUN_0083e55c = nullptr;
void* slot_0083e568_0 = nullptr; // table +0x0c (never dispatched here)
void* PTR_FUN_0083e56c = reinterpret_cast<void*>(&test_eastl_allocate);

static void init_batch_cursors(std::uint8_t* vertices, std::uint8_t* indices) {
    DAT_00cf19f0 = vertices;
    DAT_00cf19f4 = indices;
    // Pre-seed live cursors with garbage: a fresh batch must rewind them.
    DAT_00cf19f8 = vertices + 0x123;
    DAT_00cf19fc = reinterpret_cast<void*>(address_of(indices) + 0x45);
    DAT_00cf1a00 = nullptr;
    DAT_00cf1a08 = nullptr;
}

int main() {
    std::setvbuf(stdout, nullptr, _IONBF, 0); // keep output across a crash
    fake_device_vtable[0x68 / 4] = reinterpret_cast<void*>(&fake_create_vertex);
    fake_device_vtable[0x6c / 4] = reinterpret_cast<void*>(&fake_create_index);
    fake_buffer_vtable[0x2c / 4] = reinterpret_cast<void*>(&fake_lock);
    fake_buffer_vtable[0x30 / 4] = reinterpret_cast<void*>(&fake_unlock);
    DAT_00cf1944 = &fake_device;
    // ---- 0x01003ce0 stride table ------------------------------------------
    std::uint32_t axis_mask = 0;
    std::uint32_t stride = 0;
    hp6::render::compute_vertex_format_stride(0x2 | 0x10, &axis_mask, &stride);
    expect("stride_format_12_is_0x10", stride == 0x10 && axis_mask == (0x2 | 0x10));
    hp6::render::compute_vertex_format_stride(
        0x2 | 0x1000 | 0x8 | 0x100 | 0x10000 | 0x8000, &axis_mask, &stride);
    expect("stride_full_mask_sum", stride == 0x3c);
    DAT_008663d5 = 0;
    hp6::render::compute_vertex_format_stride(0x20000, &axis_mask, &stride);
    expect("stride_0x20000_quality0_is_8", stride == 8);
    DAT_008663d5 = 1;
    hp6::render::compute_vertex_format_stride(0x20000, &axis_mask, &stride);
    expect("stride_0x20000_quality1_is_4", stride == 4);
    DAT_008663d5 = 0;

    // ---- one full plain-mode layout through the chain ---------------------
    alignas(16) std::uint8_t vertex_scratch[0x400];
    alignas(16) std::uint8_t index_scratch[0x100];
    alignas(16) std::uint8_t pool_storage[0x400];
    DAT_00cf18e4 = new std::int32_t[3];
    DAT_00cf18e4[0] = static_cast<std::int32_t>(address_of(pool_storage));
    DAT_00cf18e4[1] = 0x400;
    DAT_00cf18e4[2] = static_cast<std::int32_t>(address_of(pool_storage));
    init_batch_cursors(vertex_scratch, index_scratch);

    alignas(16) static std::uint8_t region_block[0x10000];
    DAT_00967218 = region_block;
    alignas(16) static std::uint8_t command_arena[0x400];
    DAT_0096b48c = command_arena;
    static std::uint8_t normal_descriptor[0x30] = {};
    *reinterpret_cast<std::uint16_t*>(normal_descriptor + 0x22) = 3;
    DAT_009970d8[6] = normal_descriptor; // +0x14 mode 0 -> normal queueing
    DAT_0089cbfe = static_cast<char>(1u << 4);
    const float identity[16] = {1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1};
    const float scale_by_2[16] = {2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 1};
    alignas(16) static float context_area[0x50];
    std::memcpy(context_area, identity, 0x40);
    // rhs = the context's second matrix at BYTE offset +0x40 (= floats 16).
    std::memcpy(context_area + 16, identity, 0x40);
    float expected_product[16];
    hp6::math::multiply_matrix4x4(expected_product, scale_by_2, identity);

    hp6::presentation::RenderTextDescriptor descriptor = {};
    descriptor.flags = 0; // plain mode
    descriptor.matrix = scale_by_2;
    descriptor.rect_x_left = 0.0f;
    descriptor.rect_x_right = 100.0f;
    descriptor.rect_y_a = 0.0f;
    descriptor.rect_y_b = 100.0f;
    descriptor.x_left = 10.0f;
    descriptor.x_right = 30.0f;
    descriptor.y_a = 20.0f;
    descriptor.y_b = 40.0f;
    descriptor.u_left = 0x0000;
    descriptor.u_right = 0x1000;
    descriptor.v_b = 0x0000;
    descriptor.v_a = 0x1000;
    descriptor.default_bucket = 6;
    descriptor.value_08 = 0x5a5a;

    hp6::presentation::layout_text_descriptor(context_area, descriptor);

    // BatchCommand drawn from the DAT_00cf18e4 bump pool.
    expect("pool_current_advanced_0x30",
           DAT_00cf18e4[2] ==
               static_cast<std::int32_t>(address_of(pool_storage)) + 0x30);
    const auto* command = pool_storage;
    expect("cmd_markers_0x50_0x48", command[0] == 0x50 && command[1] == 0x48);
    expect("cmd_marker8_0x12",
           *reinterpret_cast<const std::uint32_t*>(command + 8) == 0x12);
    // 0x01003ce0 writes the format mask into DAT_00cf1a14 and the stride
    // into DAT_00cf1a18; begin stores +0x18 = DAT_00cf1a14 (the mask, so
    // 0x12 stays) and +0x24 = the stride's low byte (0x10).
    expect("cmd_stride_overwrote_18",
           *reinterpret_cast<const std::uint32_t*>(command + 0x18) == 0x12);
    expect("cmd_index_count_22_finalized",
           *reinterpret_cast<const std::uint16_t*>(command + 0x22) == 6);
    expect("cmd_stride_byte_24_is_0x10", command[0x24] == 0x10);
    expect("cmd_index_passes_1e",
           *reinterpret_cast<const std::uint16_t*>(command + 0x1e) == 2);
    expect("cmd_vertex_count_26_finalized",
           *reinterpret_cast<const std::uint16_t*>(command + 0x26) == 4);
    expect("cmd_dynamic_buffer_handles",
           *reinterpret_cast<const std::uint32_t*>(command + 0x10) == 0x80000000u &&
           *reinterpret_cast<const std::uint32_t*>(command + 0x14) == 0x80000000u);

    // Packed vertex output: 4 records x 0x10 (pos XYZ + one packed u16-pair
    // dword). Vertex rows: (xL,yA)(xR,yA)(xL,yB)(xR,yB) with the (u_lo,u_hi)
    // pair (u_left,v_a)(u_right,v_a)(u_left,v_b)(u_right,v_b).
    const float x_values[4] = {10.0f, 30.0f, 10.0f, 30.0f};
    const float y_values[4] = {20.0f, 20.0f, 40.0f, 40.0f};
    const std::uint16_t uv_pairs[4][2] = {
        {0x0000, 0x1000}, {0x1000, 0x1000}, {0x0000, 0x0000}, {0x1000, 0x0000}};
    bool records_ok = true;
    for (std::uint32_t i = 0; i < 4; ++i) {
        const auto* rec =
            reinterpret_cast<const std::uint32_t*>(vertex_scratch + i * 0x10);
        records_ok = records_ok &&
                     rec[0] == bits_of(x_values[i]) &&
                     rec[1] == bits_of(y_values[i]) &&
                     rec[2] == 0 &&
                     rec[3] == (static_cast<std::uint32_t>(uv_pairs[i][0]) |
                                (static_cast<std::uint32_t>(uv_pairs[i][1]) << 16));
    }
    expect("vertex_records_packed", records_ok);
    // Fresh-batch rewind took the pre-seeded garbage cursor to the base,
    // then the four 0x10-byte vertices advanced it by 0x40.
    expect("vertex_cursor_rewound_then_0x40",
           DAT_00cf19f8 == vertex_scratch + 0x40);
    expect("vertex_copied_into_driver_buffer",
           fake_buffer_count == 2 &&
           std::memcmp(fake_buffer_data[0], vertex_scratch, 0x40) == 0);

    // Indices {0,1,2, 1,3,2} through the DAT_00cf1a08 target; its target
    // variable advanced by 0xc.
    expect("index_cursor_advanced_0xc",
           address_of(DAT_00cf19fc) == address_of(index_scratch) + 0xc);
    expect("indices_copied_into_driver_buffer",
           std::memcmp(fake_buffer_data[1], index_scratch, 0xc) == 0 &&
           fake_lock_count == 2);
    const auto* written_indices =
        reinterpret_cast<const std::uint16_t*>(address_of(index_scratch));
    expect("index_pattern_flat_quad",
           written_indices[0] == 0 && written_indices[1] == 1 &&
               written_indices[2] == 2 && written_indices[3] == 1 &&
               written_indices[4] == 3 && written_indices[5] == 2);

    // Matrix product: lands in the 0x10-aligned bump slot at command_arena;
    // rhs = transform_context + 0x40 (the context's second matrix).
    const auto* item = command_arena;
    bool product_ok = true;
    for (std::uint32_t i = 0; i < 16; ++i) {
        product_ok = product_ok &&
                     *reinterpret_cast<const std::uint32_t*>(item + i * 4) ==
                         bits_of(expected_product[i]);
    }
    expect("aligned_matrix_is_lhs_x_rhs", product_ok);
    // Packet block: the queued item IS the packet (at aligned + 0x40); it
    // carries {matrix pointer, 0x8028, command, 0, descriptor.value_08} and
    // the dwords 5..8 the original leaves zeroed before the 0x24 copy.
    expect("packet_flag_0x8028",
           *reinterpret_cast<const std::uint32_t*>(item + 0x44) == 0x8028);
    expect("packet_cmd_pointer",
           *reinterpret_cast<const std::uint32_t*>(item + 0x48) ==
               address_of(command));
    expect("packet_zero_dword",
           *reinterpret_cast<const std::uint32_t*>(item + 0x4c) == 0);
    expect("packet_value_08",
           *reinterpret_cast<const std::uint32_t*>(item + 0x50) == 0x5a5a);
    expect("packet_tail_zeroed",
           *reinterpret_cast<const std::uint32_t*>(item + 0x54) == 0 &&
               *reinterpret_cast<const std::uint32_t*>(item + 0x5c) == 0 &&
               *reinterpret_cast<const std::uint32_t*>(item + 0x60) == 0);
    // Queue: bucket 6's first item (the packet block) lands at both slots
    // (normal mode; the tail slot points at the newly queued item).
    auto** tail = reinterpret_cast<void**>(region_block + 6 * 4);
    auto** head = reinterpret_cast<void**>(region_block + 0x8000 + 6 * 4);
    expect("bucket6_tail_is_item", *tail == item + 0x40);
    expect("bucket6_head_is_item", *head == item + 0x40);
    expect("region_descriptor_byte_set", DAT_009978e0[3] == 0x3f);
    expect("bucket_mask_or", (DAT_0098c690[6] & 0x10u) != 0);
    // The 0x8028 flag selects the 0x24-byte packet copy: bump = aligned
    // (matrix 0x40) + 0x24.
    expect("bump_after_submit", DAT_0096b48c == command_arena + 0x64);

    // ---- clip gate + UV lerp behavior -------------------------------------
    {
        alignas(0x20) hp6::presentation::LayoutRun runs_store[4];
        hp6::presentation::RunBuffer runs;
        runs.begin = runs_store;
        runs.cursor = runs_store;
        runs.cap_end = runs_store + 4;
        runs.reserved_0c = nullptr;
        runs.protected_begin = runs_store;
        hp6::presentation::RenderTextDescriptor region_descriptor = {};
        region_descriptor.rect_x_left = 0.0f;
        region_descriptor.rect_x_right = 10.0f;
        region_descriptor.rect_y_a = 0.0f;
        region_descriptor.rect_y_b = 10.0f;

        // Fully inside: gate stays 0, u16s untouched.
        hp6::presentation::LayoutRun inside = {};
        inside.x_left = 2; inside.x_right = 6;
        inside.y_a = 2; inside.y_b = 8;
        inside.u_left = 0x100; inside.u_right = 0x800;
        inside.v_b = 0x200; inside.v_a = 0x100;
        *runs.cursor = inside;
        runs.cursor = runs_store + 1;
        hp6::presentation::clip_runs_to_region(runs, region_descriptor);
        expect("clip_inside_gate_0",
               reinterpret_cast<const std::uint8_t*>(&runs_store[0].gate)[0] == 0 &&
                   runs_store[0].u_left == 0x100 &&
                   runs_store[0].u_right == 0x800);

        // Fully outside: gate 1, u16s untouched.
        hp6::presentation::LayoutRun outside = {};
        outside.x_left = 20; outside.x_right = 30;
        outside.y_a = 2; outside.y_b = 8;
        outside.u_left = 0x100; outside.u_right = 0x800;
        outside.v_b = 0x200; outside.v_a = 0x100;
        *runs.cursor = outside;
        runs.cursor = runs_store + 2;
        hp6::presentation::clip_runs_to_region(runs, region_descriptor);
        expect("clip_outside_gate_1",
               reinterpret_cast<const std::uint8_t*>(&runs_store[1].gate)[0] == 1 &&
                   runs_store[1].u_left == 0x100);

        // Partial x-left overlap: rect left moved to 4.0 for this case;
        // ratio (4-2)/(6-2) over span (uint)0x800-(uint)0x100 = 0x700 ->
        // trunc(0.5*0x700 + 0x100) = 0x480.
        region_descriptor.rect_x_left = 4.0f;
        hp6::presentation::LayoutRun partial = {};
        partial.x_left = 2; partial.x_right = 6;
        partial.y_a = 2; partial.y_b = 8;
        partial.u_left = 0x100; partial.u_right = 0x800;
        partial.v_b = 0x200; partial.v_a = 0x100;
        *runs.cursor = partial;
        runs.cursor = runs_store + 3;
        hp6::presentation::clip_runs_to_region(runs, region_descriptor);
        expect("clip_partial_left_u_lerp",
               runs_store[2].u_left == 0x480 && runs_store[2].x_left == 4.0f &&
                   runs_store[2].u_right == 0x800);

        // Partial y-bottom overlap only (rect left back to 0.0): ratio
        // (10-2)/(20-2) over span (uint)0x200-(uint)0x100 = 0x100, base
        // 0x100 -> trunc(8/18*256 + 256) = 369; then y_b = 10; y_top
        // untouched.
        region_descriptor.rect_x_left = 0.0f;
        hp6::presentation::LayoutRun bottom = {};
        bottom.x_left = 2; bottom.x_right = 6;
        bottom.y_a = 2; bottom.y_b = 20;
        bottom.u_left = 0x100; bottom.u_right = 0x800;
        bottom.v_b = 0x200; bottom.v_a = 0x100;
        *runs.cursor = bottom;
        runs.cursor = runs_store + 4;
        hp6::presentation::clip_runs_to_region(runs, region_descriptor);
        expect("clip_bottom_v_lerp",
               runs_store[3].v_b == 369 && runs_store[3].y_b == 10.0f &&
                   runs_store[3].v_a == 0x100 && runs_store[3].y_a == 2.0f);

        // Both y clamps in one run; the top clamp's divisor must see the
        // post-clamp y_b AND the span reads the post-clamp v_b (asm
        // 005e978d re-reads word[EAX+0x1c]): y_a=-2, y_b=12, rect y
        // [0,10]: bottom -> trunc(12/14*0x100 + 0x100) = 475, y_b=10; top
        // -> trunc(2/12*(475-256) + 256) = 292, y_a=0. The written run
        // reuses index 3 (cursor is rewound to it first).
        hp6::presentation::LayoutRun both = {};
        both.x_left = 2; both.x_right = 6;
        both.y_a = -2; both.y_b = 12;
        both.u_left = 0x100; both.u_right = 0x800;
        both.v_b = 0x200; both.v_a = 0x100;
        runs.cursor = runs_store + 3;
        *runs.cursor = both;
        runs.cursor = runs_store + 4;
        hp6::presentation::clip_runs_to_region(runs, region_descriptor);
        expect("clip_y_pair_sequential",
               runs_store[3].v_b == 475 && runs_store[3].v_a == 292 &&
                   runs_store[3].y_a == 0.0f && runs_store[3].y_b == 10.0f);
    }

    // ---- tag path: measure, lookup, bucket resolution, run emission -------
    {
        alignas(16) static std::uint8_t registry_block[0x400];
        DAT_00fcaec0 = registry_block;
        void** registry = reinterpret_cast<void**>(registry_block + 0x14);
        *reinterpret_cast<void**>(registry_block + 0x14 + 0xc) = nullptr; // no transform
        static char tag_name[] = "FE:BODY";
        alignas(16) static std::uint8_t node[0x100] = {};
        registry[0] = node;                       // registry+0 = first node
        reinterpret_cast<void**>(node)[0] = registry;  // sentinel foot loop
        reinterpret_cast<void**>(node)[2] = tag_name;  // node+8 = name pointer

        void* tag_handle = reinterpret_cast<void**>(node) + 2;
        static const char match_name[] = "FE";
        auto* tag_bytes = static_cast<std::uint8_t*>(tag_handle);
        *reinterpret_cast<std::uint32_t*>(tag_bytes + 0x10) =
            static_cast<std::uint32_t>(address_of(match_name)); // entry name
        *reinterpret_cast<std::uint32_t*>(tag_bytes + 0x14) =
            0x00000000; // match_word in the low half
        *reinterpret_cast<std::uint32_t*>(tag_bytes + 0x18) = 2; // match_dword
        *reinterpret_cast<std::uint32_t*>(tag_bytes + 0x90) = 1; // count
        *reinterpret_cast<std::uint16_t*>(tag_bytes + 0x7c) = 0x1234;

        expect("bucket_resolves_match",
               hp6::presentation::resolve_run_font_bucket(
                   address_of(tag_handle), "FE", 0, 2) == 0x1234);
        expect("bucket_fallback_word",
               hp6::presentation::resolve_run_font_bucket(
                   address_of(tag_handle), "NOPE", 0, 2) == 0x0bad);
        expect("bucket_word_mismatch",
               hp6::presentation::resolve_run_font_bucket(
                   address_of(tag_handle), "FE", 1, 2) == 0x0bad);

        // measure_tag_body_length terminates at `\]`, counts plain chars.
        hp6::presentation::RenderTextDescriptor body_descriptor = {};
        static wchar_t body_text[] = L"a\\]bc";
        body_descriptor.text = body_text;
        expect("measure_counts_then_terminates",
               hp6::presentation::measure_tag_body_length(body_descriptor, 0) == 1);
        static wchar_t unterminated[] = L"abc";
        body_descriptor.text = unterminated;
        expect("measure_unterminated_is_zero",
               hp6::presentation::measure_tag_body_length(body_descriptor, 0) == 0);

        // Full text-tag layout: "\[FE\]x" -> run(tag) + run(plain glyph).
        // Queue environment: the tag run's bucket (0x1234 from the matcher)
        // must be visible in the queued item, so its descriptor goes to the
        // indexed mode table slot 1 (_Dst tables + bucket copy at +0x24).
        // The plain-glyph run's bucket in text mode is DAT_00d714b8's word
        // (0x99) — a normal-mode descriptor.
        static std::uint8_t tag_descriptor[0x30] = {};
        *reinterpret_cast<std::uint32_t*>(tag_descriptor + 0x14) =
            0x40000000u | (1u << 10); // indexed mode, table slot 1
        *reinterpret_cast<std::uint16_t*>(tag_descriptor + 0x22) = 4;
        DAT_009970d8[0x1234] = tag_descriptor;
        DAT_00d714b8 = 0x99;
        static std::uint8_t glyph_descriptor[0x30] = {};
        *reinterpret_cast<std::uint16_t*>(glyph_descriptor + 0x22) = 5;
        DAT_009970d8[0x99] = glyph_descriptor;
        DAT_00998580 = 0x2a;
        DAT_0098c690[0x1234] = 0;
        DAT_0098c690[0x99] = 0;

        static wchar_t full_text[] = L"\\[FE\\]x";
        const float corner_left = 48.0f;
        const float corner_other = 20.0f;
        std::memcpy(&DAT_00728004, &corner_left, 4);
        std::memcpy(&DAT_0082ae34, &corner_other, 4);
        alignas(16) std::uint8_t vertex_scratch2[0x400];
        alignas(16) std::uint8_t index_scratch2[0x100];
        DAT_00cf18e4[2] = static_cast<std::int32_t>(address_of(pool_storage)) + 0x30;
        init_batch_cursors(vertex_scratch2, index_scratch2);

        hp6::presentation::RenderTextDescriptor text_descriptor = {};
        text_descriptor.flags = 4;
        text_descriptor.text = full_text;
        text_descriptor.matrix = scale_by_2;
        text_descriptor.rect_x_left = 0.0f;
        text_descriptor.rect_x_right = 100.0f;
        text_descriptor.rect_y_a = 0.0f;
        text_descriptor.rect_y_b = 100.0f;
        text_descriptor.x_left = 5.0f;
        text_descriptor.x_right = 15.0f;
        text_descriptor.y_a = 3.0f;
        text_descriptor.y_b = 9.0f;
        text_descriptor.u_left = 0x0000;
        text_descriptor.u_right = 0x1000;
        text_descriptor.v_b = 0x0000;
        text_descriptor.v_a = 0x1000;
        text_descriptor.default_bucket = 6;
        text_descriptor.match_name = "FE";
        text_descriptor.match_data_48 = 0;
        text_descriptor.match_dword = 2;

        DAT_0096b48c = command_arena + 0x24;
        hp6::presentation::layout_text_descriptor(context_area, text_descriptor);

        expect("text_mode_pool_two_commands",
               DAT_00cf18e4[2] ==
                   static_cast<std::int32_t>(address_of(pool_storage)) + 0x90);
        const auto* rec0 = reinterpret_cast<const std::uint32_t*>(vertex_scratch2);
        // Per-run rewind: 0x005e9820 provisions each run's command with
        // +0x26 = 0, so 0x01003b20 rewinds both scratch cursors for every
        // run; the LAST run's packed records live at scratch+0 and the tag
        // run's records are overwritten by the glyph run's. The tag run's
        // own staging is checked separately by the tag-only case below.
        const auto* rec_glyph =
            reinterpret_cast<const std::uint32_t*>(vertex_scratch2);
        // Glyph row 1 = (x_left, y_a) with the shared corner constants:
        // x_left and y_a both read DAT_00728004.
        expect("plain_glyph_run_corner_constants",
               rec_glyph[0] == DAT_00728004 && rec_glyph[1] == DAT_00728004 &&
                   rec_glyph[2] == 0);
        expect("glyph_row1_uv_pair",
               rec_glyph[3] == (0x00000000u | (0x1000u << 16)));
        // Row 2 (x_right, y_a) = (corner_other, corner_left), uv (0x1000, 0x1000).
        const auto* rec_glyph_row2 =
            reinterpret_cast<const std::uint32_t*>(vertex_scratch2 + 0x10);
        expect("glyph_row2_constants",
               rec_glyph_row2[0] == DAT_0082ae34 &&
                   rec_glyph_row2[1] == DAT_00728004 &&
                   rec_glyph_row2[3] == (0x1000u | (0x1000u << 16)));

        // Item 1 (tag run): the 0x10-aligned matrix slot sits at +0x30
        // (the 0x24 seed feeds the align), and the queued item is the
        // 5-dword packet block that follows it (item = aligned + 0x40).
        // The indexed-mode copy grows by 0x2c with the bucket/flags/mode
        // tail fields.
        const auto* tag_item = command_arena + 0x70;
        expect("tag_item_product",
               *reinterpret_cast<const std::uint32_t*>(tag_item - 0x40) ==
                   bits_of(expected_product[0]));
        expect("tag_item_bucket_from_matcher",
               *reinterpret_cast<const std::int16_t*>(tag_item + 0x24) == 0x1234);
        expect("tag_item_flags_shift",
               *reinterpret_cast<const std::int16_t*>(tag_item + 0x26) == 1);
        expect("tag_item_region_mode",
               *reinterpret_cast<const std::int32_t*>(tag_item + 0x28) == 0x2a);
        expect("tag_item_indexed_tail",
               _Dst_00b10ce8[1] == const_cast<void*>(static_cast<const void*>(tag_item)));
        expect("tag_item_indexed_head",
               _Dst_00966fd8[1] == const_cast<void*>(static_cast<const void*>(tag_item)));
        expect("tag_item_bump",
               DAT_0096b48c == command_arena + 0xe0 + 0x24);
        // Item 2 (plain glyph, bucket 0x99) in normal mode in the region
        // block; the head slot got the FIRST normal item for that bucket.
        // Matrix at the next 0x10-aligned slot (+0x9c -> +0xa0), item at
        // +0xe0, packet copy of 0x24 bytes.
        const auto* glyph_item = command_arena + 0xe0;
        auto** glyph_tail = reinterpret_cast<void**>(region_block + 0x99 * 4);
        auto** glyph_head = reinterpret_cast<void**>(region_block + 0x8000 + 0x99 * 4);
        expect("glyph_item_normal_tail", *glyph_tail == glyph_item);
        expect("glyph_item_normal_head", *glyph_head == glyph_item);
        expect("glyph_item_descriptor_byte", DAT_009978e0[5] == 0x3f);

        // Failed lookup: "\[ZZ\]y" -> no tag run; the loop resumes after
        // the opener and every remaining plain character emits a run
        // (three 0x30-byte commands, three glyph items queued at 0x99).
        static wchar_t failed_text[] = L"\\[ZZ\\]y";
        text_descriptor.text = failed_text;
        init_batch_cursors(vertex_scratch2, index_scratch2);
        DAT_00cf18e4[2] = static_cast<std::int32_t>(address_of(pool_storage));
        DAT_0096b48c = command_arena + 0x24 + 0x2c + 0x24;
        hp6::presentation::layout_text_descriptor(context_area, text_descriptor);
        expect("failed_tag_three_plain_runs",
               DAT_00cf18e4[2] ==
                   static_cast<std::int32_t>(address_of(pool_storage)) + 0x90);
        const auto* rec_failed =
            reinterpret_cast<const std::uint32_t*>(vertex_scratch2);
        // Last glyph run ('y'): x_left and y_a both carry the corner-left
        // constant (y_a copies the same constant in the plain path).
        expect("failed_tag_plains_use_constants",
               rec_failed[0] == DAT_00728004 && rec_failed[1] == DAT_00728004);
        auto** failed_tail = reinterpret_cast<void**>(region_block + 0x99 * 4);
        // Items: seed 0x74 -> matrix +0x80/item +0xc0 (bump 0xe4), matrix
        // +0xf0/item +0x130 (bump 0x154), matrix +0x160/item +0x1a0.
        expect("failed_tag_three_items_queued",
               *failed_tail == command_arena + 0x1a0);

        // Tag-only layout: a single run, so the per-run rewind leaves the
        // TAG run's own packed records at scratch+0 (the descriptor's corner
        // floats staged by the tag path).
        static wchar_t tag_only_text[] = L"\\[FE\\]";
        text_descriptor.text = tag_only_text;
        init_batch_cursors(vertex_scratch2, index_scratch2);
        DAT_00cf18e4[2] = static_cast<std::int32_t>(address_of(pool_storage));
        DAT_0096b48c = command_arena + 0x200;
        hp6::presentation::layout_text_descriptor(context_area, text_descriptor);
        expect("tag_only_pool_one_command",
               DAT_00cf18e4[2] ==
                   static_cast<std::int32_t>(address_of(pool_storage)) + 0x30);
        const auto* rec_tag = reinterpret_cast<const std::uint32_t*>(vertex_scratch2);
        expect("tag_run_uses_descriptor_corners",
               rec_tag[0] == bits_of(5.0f) && rec_tag[1] == bits_of(3.0f) &&
                   rec_tag[2] == 0);
        expect("tag_run_uv_pair_from_descriptor",
               rec_tag[3] == (0x0000u | (0x1000u << 16)));
    }

    // ---- insert_run growth path (scratch pool exhausted) ------------------
    {
        alignas(0x20) hp6::presentation::LayoutRun full_store[2];
        hp6::presentation::RunBuffer full;
        full.begin = full_store;
        full.cursor = full_store;
        full.cap_end = full_store + 2;
        full.reserved_0c = nullptr;
        full.protected_begin = full_store;
        hp6::presentation::LayoutRun seed = {};
        seed.x_left = 1.0f;
        *full.cursor++ = seed;
        hp6::presentation::LayoutRun seed2 = {};
        seed2.x_left = 2.0f;
        *full.cursor++ = seed2;
        hp6::presentation::LayoutRun fresh = {};
        fresh.x_left = 3.0f;
        hp6::presentation::insert_run(full, full.cursor, fresh);
        expect("insert_run_grew_and_appended",
               full.begin != full_store && full.cursor == full.begin + 3 &&
                   full.cap_end == full.begin + 4 &&
                   full.begin[0].x_left == 1.0f && full.begin[1].x_left == 2.0f &&
                   full.begin[2].x_left == 3.0f);
        // protected_begin (= full_store) kept the old block from being
        // released; the allocator got exactly the doubled byte count
        // (2 elements -> 4 -> 4*0x20) with the context contract intact.
        expect("insert_run_release_guard", stub_freed_stack_scratch == false);
        expect("insert_run_grow_alloc_context",
               stub_error_context_matches == 1);
    }

    // Language-menu key -> bitmap descriptor -> text layout -> D3D9 pool.
    // This is the first connected slice across the two formerly isolated
    // reconstructions; the per-frame presentation caller is still separate.
    {
        hp6::presentation::update_localized_text(
            0x00cf8168u, "GUI_LANGUAGEMENU0", L"A");
        alignas(16) std::uint8_t bitmap[0x59]{};
        bitmap[0x58] = 1;
        bitmap[0x28] = 0x10; // source bit 4 -> descriptor text mode
        alignas(16) std::uint8_t parent[0x50]{};
        std::memcpy(parent + 0x10, identity, sizeof(identity));
        *reinterpret_cast<std::uint32_t*>(bitmap + 0x04) = address_of(parent);
        const hp6::presentation::BitmapCoordinateSpace spaces[] = {
            {"WORLD", 0, 0}};
        const char* key = "GUI_LANGUAGEMENU0";
        *reinterpret_cast<std::uint32_t*>(bitmap + 0x2c) = address_of(key);
        *reinterpret_cast<float*>(bitmap + 0x10) = 1;
        *reinterpret_cast<float*>(bitmap + 0x14) = 1;
        *reinterpret_cast<float*>(bitmap + 0x18) = 1;
        *reinterpret_cast<float*>(bitmap + 0x48) = -1;
        *reinterpret_cast<float*>(bitmap + 0x4c) = 2;
        *reinterpret_cast<float*>(bitmap + 0x50) = -1;
        *reinterpret_cast<float*>(bitmap + 0x54) = 2;
        DAT_00728004 = bits_of(0.0f);
        DAT_0082ae34 = bits_of(1.0f);
        DAT_00d714b8 = 6;
        DAT_00cf18e4[2] = DAT_00cf18e4[0];
        DAT_0096b48c = command_arena;
        init_batch_cursors(vertex_scratch, index_scratch);
        const auto* driver_cursor_before =
            *reinterpret_cast<std::uint8_t**>(DAT_00baa170 + 4);
        hp6::presentation::BitmapRenderQuad quad;
        const bool visible = hp6::presentation::build_bitmap_render_quad_mode2(
            bitmap, spaces, quad);
        expect("language_bitmap_visible_and_resolved",
               visible && quad.descriptor.text != nullptr &&
               quad.descriptor.text[0] == L'A');
        if (visible) {
            expect("language_bitmap_submitted_to_layout",
                   hp6::presentation::submit_bitmap_render_quad_mode2(
                       bitmap, context_area, spaces));
            expect("language_quad_command_has_geometry",
                   *reinterpret_cast<const std::uint16_t*>(pool_storage + 0x26) == 4 &&
                   *reinterpret_cast<const std::uint16_t*>(pool_storage + 0x22) == 6);
            expect("language_quad_vertices_reached_d3d_buffer",
                   std::memcmp(driver_cursor_before, vertex_scratch, 0x40) == 0);
        }
    }

    if (failures != 0) {
        std::printf("%d failures\n", failures);
        return 1;
    }
    std::printf("all ok\n");
    return 0;
}
