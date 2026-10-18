// Local behavior checks for batch render_reset_056b: the Direct3D 9
// device-reset / shader-recreation chain reached from WinMain's message
// loop (see architecture/startup_path.md). Game-space symbols and a fake
// D3D9-shaped vtable get test-local definitions.
#include "render/DeviceReset.h"

#include <cstdint>
#include <cstdio>
#include <cstring>

#ifdef _MSC_VER
#define HP6_STDCALL __stdcall
#else
#define HP6_STDCALL
#endif

std::int32_t DAT_008663d8 = 0;
std::int32_t DAT_00cf19d0 = 0;
void* DAT_00cf1944 = nullptr;
std::uint32_t DAT_00cf19d4 = 0;
std::uint8_t DAT_00865286 = 0;
void* DAT_00c75a7c = nullptr;

struct ShaderSlot {
    std::uint32_t bytecode;
    std::uint32_t shader;
};
ShaderSlot DAT_00a68820[4] = {};
std::uint32_t DAT_00cf1a30 = 0;

std::uint32_t DAT_00cf1a38 = 0;
std::uint8_t DAT_00f38b10[0x600 * 0xc4] = {};
std::int32_t DAT_00cf19bc = 0;

std::uint32_t DAT_00cf1964 = 0;
std::uint8_t DAT_008663d5 = 0;
std::int32_t DAT_008928c0[256] = {};
std::int32_t DAT_008663b8 = 0;
std::uint32_t DAT_008663dc = 0;
std::int32_t DAT_00cf1974 = 0;

namespace {
int failures = 0;
void expect(const char* label, bool ok) {
    if (!ok) { ++failures; std::printf("FAIL %s\n", label); }
}
std::uint32_t address(const void* value) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(value));
}

// ---- fake COM-shaped objects -----------------------------------------
int release_calls = 0;
void* last_released = nullptr;
void HP6_STDCALL fake_release(void* self) {
    ++release_calls;
    last_released = self;
}
void* shader_object_vtable[4] = {nullptr, nullptr, reinterpret_cast<void*>(&fake_release),
                                 nullptr};
struct FakeShaderObject {
    void* vtable = shader_object_vtable;
    std::uint32_t tag;
};

int create_vs_calls = 0;
std::uint32_t last_vs_bytecode = 0;
void* last_vs_slot = nullptr;
std::uint32_t next_vs_handle = 0x9000;
std::int32_t HP6_STDCALL fake_create_vertex_shader(void* device, std::uint32_t bytecode,
                                                   void* out_slot) {
    (void)device;
    ++create_vs_calls;
    last_vs_bytecode = bytecode;
    last_vs_slot = out_slot;
    *reinterpret_cast<std::uint32_t*>(out_slot) = ++next_vs_handle;
    return 0;
}

int create_ps_calls = 0;
std::uint32_t last_ps_bytecode = 0;
std::int32_t HP6_STDCALL fake_create_pixel_shader(void* device, std::uint32_t bytecode,
                                                  void* out_slot) {
    (void)device;
    ++create_ps_calls;
    last_ps_bytecode = bytecode;
    *reinterpret_cast<std::uint32_t*>(out_slot) = 0xA000;
    return 0;
}

int create_vb_calls = 0;
std::uint32_t last_vb_length = 0, last_vb_usage = 0, last_vb_fvf = 0, last_vb_pool = 0;
std::int32_t HP6_STDCALL fake_create_vertex_buffer(void* device, std::uint32_t length,
                                                   std::uint32_t usage, std::uint32_t fvf,
                                                   std::uint32_t pool, void* out_slot,
                                                   std::uint32_t shared_handle) {
    (void)device;
    (void)shared_handle;
    ++create_vb_calls;
    last_vb_length = length;
    last_vb_usage = usage;
    last_vb_fvf = fvf;
    last_vb_pool = pool;
    *reinterpret_cast<void**>(out_slot) = reinterpret_cast<void*>(0x5000);
    return 0;
}

std::uint32_t available_texture_mem_bytes = 0;
std::uint32_t HP6_STDCALL fake_get_available_texture_mem(void* device) {
    (void)device;
    return available_texture_mem_bytes;
}

void* device_vtable[112] = {};
struct FakeDevice {
    void* vtable = device_vtable;
};
FakeDevice fake_device;

} // namespace

int main() {
    using namespace hp6::render;
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    device_vtable[0x68 / sizeof(void*)] = reinterpret_cast<void*>(&fake_create_vertex_buffer);
    device_vtable[0x16c / sizeof(void*)] = reinterpret_cast<void*>(&fake_create_vertex_shader);
    device_vtable[0x1a8 / sizeof(void*)] = reinterpret_cast<void*>(&fake_create_pixel_shader);
    DAT_00cf1944 = &fake_device;

    // ---- reset_hook_placeholder (00650d80) --------------------------------
    reset_hook_placeholder();

    // ---- compute_dynamic_table_size (0063b4a0) ----------------------------
    DAT_008663d8 = 2;
    DAT_00cf19d0 = 0x10;
    expect("table_size", compute_dynamic_table_size() == 2 * 0x584 + 0x454 + 0x10);

    // ---- create_dynamic_vertex_buffer (00648e10) --------------------------
    DAT_00865286 = 0;
    DAT_00cf19d4 = 0x10;
    create_dynamic_vertex_buffer();
    expect("vb_called", create_vb_calls == 1);
    expect("vb_length", last_vb_length == 0xe0);
    expect("vb_usage", last_vb_usage == (0x10u | 0x208u));
    expect("vb_fvf", last_vb_fvf == 0);
    expect("vb_pool_sysmem", last_vb_pool == 2);
    expect("vb_output", DAT_00c75a7c == reinterpret_cast<void*>(0x5000));

    DAT_00865286 = 1;
    create_dynamic_vertex_buffer();
    expect("vb_pool_default", last_vb_pool == 0);

    // ---- recreate_vertex_shaders (006525d0) -------------------------------
    FakeShaderObject shader_a{}, shader_b{};
    shader_a.tag = 1;
    shader_b.tag = 2;
    DAT_00cf1a30 = 2;
    DAT_00a68820[0] = {0x1111u, address(&shader_a)};
    DAT_00a68820[1] = {0x2222u, address(&shader_b)};
    create_vs_calls = 0;
    recreate_vertex_shaders();
    expect("recreate_release_count", release_calls == 2);
    expect("recreate_create_count", create_vs_calls == 2);
    expect("recreate_bytecode_last", last_vs_bytecode == 0x2222u);
    expect("recreate_slot_updated",
           DAT_00a68820[0].shader != address(&shader_a) &&
               DAT_00a68820[1].shader != address(&shader_b));

    // ---- release_shader_if_present (006534a0) -----------------------------
    release_calls = 0;
    last_released = nullptr;
    release_shader_if_present(0, nullptr);
    expect("release_null_noop", release_calls == 0);
    FakeShaderObject shader_c{};
    release_shader_if_present(1, &shader_c); // selector must not matter
    expect("release_called", release_calls == 1 && last_released == &shader_c);

    // ---- recreate_or_invalidate_shader (00653430) -------------------------
    alignas(16) std::uint8_t record[0x30] = {};
    create_vs_calls = 0;
    recreate_or_invalidate_shader(0x3333u, 0, 0, static_cast<std::int32_t>(address(record)));
    expect("recreate_vertex_branch", create_vs_calls == 1 && last_vs_bytecode == 0x3333u);

    *reinterpret_cast<std::uint16_t*>(record + 0x22) = 7;
    DAT_00cf1a38 = 7;
    *reinterpret_cast<std::uint32_t*>(record + 0x2c) = 0xdeadbeef;
    recreate_or_invalidate_shader(0x4444u, 0, 1, static_cast<std::int32_t>(address(record)));
    expect("recreate_clear_branch", *reinterpret_cast<std::uint32_t*>(record + 0x2c) == 0);

    *reinterpret_cast<std::uint16_t*>(record + 0x22) = 9; // mismatches DAT_00cf1a38 (7)
    create_ps_calls = 0;
    recreate_or_invalidate_shader(0x5555u, 0, 1, static_cast<std::int32_t>(address(record)));
    expect("recreate_pixel_branch", create_ps_calls == 1 && last_ps_bytecode == 0x5555u);

    // ---- recreate_shader_table (006532e0) ----------------------------------
    std::memset(DAT_00f38b10, 0, sizeof DAT_00f38b10);
    FakeShaderObject table_shader{};
    auto* entry = DAT_00f38b10 + 3 * 0xc4;
    *reinterpret_cast<std::int32_t*>(entry + 0x14) = 0;   // vertex shader type
    *reinterpret_cast<std::uint32_t*>(entry + 0x18) = 0x6666u; // bytecode / active flag
    *reinterpret_cast<std::uint32_t*>(entry + 0x1c) = 0;
    *reinterpret_cast<std::uint32_t*>(entry + 0x2c) = address(&table_shader);
    release_calls = 0;
    create_vs_calls = 0;
    recreate_shader_table();
    expect("table_release_old", release_calls == 1 && last_released == &table_shader);
    expect("table_create_new", create_vs_calls == 1 && last_vs_bytecode == 0x6666u);

    // ---- reset_dynamic_resources (00642420) --------------------------------
    // recreate_vertex_shaders() always releases whatever is currently in
    // DAT_00a68820[*].shader, so each call below must re-seed valid shader
    // objects first (the earlier recreate_vertex_shaders test already
    // overwrote those slots with fake opaque handles, not objects).
    DAT_00cf19bc = 1; // <= 2: only recreate_vertex_shaders runs
    DAT_00a68820[0] = {0x1111u, address(&shader_a)};
    DAT_00a68820[1] = {0x2222u, address(&shader_b)};
    create_vs_calls = 0;
    std::memset(DAT_00f38b10, 0, sizeof DAT_00f38b10);
    reset_dynamic_resources();
    expect("reset_shaders_only", create_vs_calls == 2); // the 2 slots from DAT_00a68820

    DAT_00cf19bc = 5; // > 2: recreate_shader_table also runs
    DAT_00a68820[0] = {0x1111u, address(&shader_a)};
    DAT_00a68820[1] = {0x2222u, address(&shader_b)};
    *reinterpret_cast<std::int32_t*>(entry + 0x14) = 0;
    *reinterpret_cast<std::uint32_t*>(entry + 0x18) = 0x7777u;
    *reinterpret_cast<std::uint32_t*>(entry + 0x1c) = 0;
    *reinterpret_cast<std::uint32_t*>(entry + 0x2c) = address(&table_shader);
    create_vs_calls = 0;
    reset_dynamic_resources();
    expect("reset_shaders_and_table", create_vs_calls == 3); // 2 slots + the table entry

    // ---- cache_available_texture_memory / adjust_texture_quality_for_memory
    // (0064aa30 / 0063b4c0+01011a50) -----------------------------------------
    device_vtable[0x10 / sizeof(void*)] =
        reinterpret_cast<void*>(&fake_get_available_texture_mem);
    DAT_008663d5 = 0;
    DAT_008928c0[0] = 4;
    DAT_008663b8 = 0;
    DAT_00cf1974 = 0;

    available_texture_mem_bytes = 10u << 0x14; // 10 "MiB"
    DAT_008663dc = 0xdeadbeefu;
    cache_available_texture_memory();
    expect("texmem_cached", DAT_00cf1964 == 10);
    expect("texmem_quality_adjusted", DAT_008663dc == 2u);

    available_texture_mem_bytes = 0;
    DAT_008663dc = 0xdeadbeefu;
    cache_available_texture_memory();
    expect("texmem_zero_noop", DAT_00cf1964 == 0 && DAT_008663dc == 0xdeadbeefu);

    available_texture_mem_bytes = 0x21u << 0x14; // above the 0x20 MiB cap
    DAT_008663dc = 0xdeadbeefu;
    cache_available_texture_memory();
    expect("texmem_above_cap_noop",
           DAT_00cf1964 == 0x21 && DAT_008663dc == 0xdeadbeefu);

    if (failures != 0) {
        std::printf("device_reset_smoke: %d failure(s)\n", failures);
        return 1;
    }
    std::printf("device_reset_smoke: all checks passed\n");
    return 0;
}
