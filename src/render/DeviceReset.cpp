#include "render/DeviceReset.h"

#include <cstddef>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern std::int32_t DAT_008663d8;
extern std::int32_t DAT_00cf19d0;
extern void* DAT_00cf1944;              // IDirect3DDevice9*
extern std::uint32_t DAT_00cf19d4;
extern std::uint8_t DAT_00865286;
extern void* DAT_00c75a7c;              // ppVertexBuffer output slot

struct ShaderSlot {
    std::uint32_t bytecode;
    std::uint32_t shader;
};
extern ShaderSlot DAT_00a68820[];
extern std::uint32_t DAT_00cf1a30;

extern std::uint32_t DAT_00cf1a38;
extern std::uint8_t DAT_00f38b10[];
extern std::int32_t DAT_00cf19bc;

extern std::uint32_t DAT_00cf1964;    // cached available texture memory, in ~MiB
extern std::uint8_t DAT_008663d5;
extern std::int32_t DAT_008928c0[];   // table indexed by DAT_008663b8
extern std::int32_t DAT_008663b8; // matches RegistryConfig.cpp/WinMain.cpp's own extern type
extern std::uint32_t DAT_008663dc;
extern std::int32_t DAT_00cf1974;

namespace hp6::render {
namespace {

#ifdef _MSC_VER
using ReleaseCall = void (__stdcall*)(void*);
using CreateVertexShaderCall = std::int32_t (__stdcall*)(void*, std::uint32_t, void*);
using CreatePixelShaderCall = std::int32_t (__stdcall*)(void*, std::uint32_t, void*);
using CreateVertexBufferCall = std::int32_t (__stdcall*)(
    void*, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, void*,
    std::uint32_t);
#else
using ReleaseCall = void (*)(void*);
using CreateVertexShaderCall = std::int32_t (*)(void*, std::uint32_t, void*);
using CreatePixelShaderCall = std::int32_t (*)(void*, std::uint32_t, void*);
using CreateVertexBufferCall = std::int32_t (*)(
    void*, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t, void*,
    std::uint32_t);
#endif

void* vtable_of(void* object) {
    return *reinterpret_cast<void* const*>(object);
}

} // namespace

void reset_hook_placeholder() {}

std::int32_t compute_dynamic_table_size() {
    return DAT_008663d8 * 0x584 + 0x454 + DAT_00cf19d0;
}

// ReAgent candidate: reports/pipeline/candidates/00648e10.cpp. Corrected
// from __thiscall to __stdcall: the exported x86 pushes the device
// pointer on the stack before the indirect call rather than keeping it
// in ECX, matching STDMETHODCALLTYPE (COM calls are __stdcall on x86).
void create_dynamic_vertex_buffer() {
    const std::uint32_t pool = (DAT_00865286 != 0) ? 0u : 2u;
    const std::uint32_t usage = DAT_00cf19d4 | 0x208u;
    auto create = *reinterpret_cast<CreateVertexBufferCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x68);
    create(DAT_00cf1944, 0xe0u, usage, 0u, pool, &DAT_00c75a7c, 0u);
}

// ReAgent candidate: reports/pipeline/rejected/006525d0.cpp, with the
// same __thiscall-to-__stdcall correction as create_dynamic_vertex_buffer
// (confirmed again here: the exported x86 pushes both the shader object
// and the device pointer before their respective calls).
void recreate_vertex_shaders() {
    for (std::uint32_t i = 0; i < DAT_00cf1a30; ++i) {
        auto* object = reinterpret_cast<void*>(
            static_cast<std::uintptr_t>(DAT_00a68820[i].shader));
        auto release = *reinterpret_cast<ReleaseCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(object)) + 8);
        release(object);
    }
    for (std::uint32_t i = 0; i < DAT_00cf1a30; ++i) {
        auto create = *reinterpret_cast<CreateVertexShaderCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x16c);
        create(DAT_00cf1944, DAT_00a68820[i].bytecode, &DAT_00a68820[i].shader);
    }
}

// ReAgent's own candidate (reports/pipeline/rejected/006534a0.cpp) cast
// through void(***)(int*), reading one pointer level too many; its own
// objective reviewer flagged this independently. The exported x86 reads
// the vtable (*shader) and then the method pointer (vtable+8) directly —
// two loads total, confirmed here.
void release_shader_if_present(std::int32_t selector, void* shader) {
    (void)selector; // read but does not change either branch's behavior
    if (shader == nullptr) {
        return;
    }
    auto release = *reinterpret_cast<ReleaseCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(shader)) + 8);
    release(shader);
}

// ReAgent candidate: reports/pipeline/candidates/00653430.cpp, with the
// same __thiscall-to-__stdcall correction.
void recreate_or_invalidate_shader(std::uint32_t bytecode,
                                   std::uint32_t /*unused_param_2*/,
                                   std::int32_t shader_type,
                                   std::int32_t record_address) {
    auto* record = reinterpret_cast<std::uint8_t*>(
        static_cast<std::uintptr_t>(record_address));
    if (shader_type == 0) {
        auto create = *reinterpret_cast<CreateVertexShaderCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x16c);
        create(DAT_00cf1944, bytecode, record + 0x2c);
    } else if (static_cast<std::uint32_t>(
                   *reinterpret_cast<std::uint16_t*>(record + 0x22)) ==
               DAT_00cf1a38) {
        *reinterpret_cast<std::uint32_t*>(record + 0x2c) = 0;
    } else {
        auto create = *reinterpret_cast<CreatePixelShaderCall const*>(
            static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x1a8);
        create(DAT_00cf1944, bytecode, record + 0x2c);
    }
}

// ReAgent candidate: reports/pipeline/candidates/006532e0.cpp. Rejected
// only for raw offsets (unresolved record layout) and an evaluation-order
// caution between independent, side-effect-free reads of the same entry.
void recreate_shader_table() {
    constexpr std::uint32_t kCount = 0x600;
    constexpr std::uint32_t kStride = 0xc4;
    for (std::uint32_t i = 0; i < kCount; ++i) {
        auto* entry = DAT_00f38b10 + i * kStride;
        if (*reinterpret_cast<std::uint32_t*>(entry + 0x18) != 0) {
            release_shader_if_present(
                *reinterpret_cast<std::int32_t*>(entry + 0x14),
                reinterpret_cast<void*>(static_cast<std::uintptr_t>(
                    *reinterpret_cast<std::uint32_t*>(entry + 0x2c))));
        }
    }
    for (std::uint32_t i = 0; i < kCount; ++i) {
        auto* entry = DAT_00f38b10 + i * kStride;
        if (*reinterpret_cast<std::uint32_t*>(entry + 0x18) != 0) {
            recreate_or_invalidate_shader(
                *reinterpret_cast<std::uint32_t*>(entry + 0x18),
                *reinterpret_cast<std::uint32_t*>(entry + 0x1c),
                *reinterpret_cast<std::int32_t*>(entry + 0x14),
                static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(entry)));
        }
    }
}

// ReAgent candidate: reports/pipeline/candidates/00642420.cpp. Rejected
// only for the tail-call shape (JMP, not CALL+RET) into
// recreate_shader_table at the end; falling off the end of this void
// function is observably identical.
void reset_dynamic_resources() {
    recreate_vertex_shaders();
    if (DAT_00cf19bc > 2) {
        recreate_shader_table();
    }
}

// ReAgent FAIL overridden: rejected only for not matching the binary's
// branch polarity independently from P-code alone; re-derivation from
// the exported x86 (JZ on DAT_00cf1964==0, then JA 0x20 — i.e. the call
// happens for 0 < value <= 0x20) confirms the decompiled polarity below.
void cache_available_texture_memory() {
    using GetAvailableTextureMemCall = std::uint32_t (__stdcall*)(void*);
    auto call = *reinterpret_cast<GetAvailableTextureMemCall const*>(
        static_cast<const std::uint8_t*>(vtable_of(DAT_00cf1944)) + 0x10);
    const std::uint32_t bytes = call(DAT_00cf1944);
    DAT_00cf1964 = bytes >> 0x14;
    if (DAT_00cf1964 != 0 && DAT_00cf1964 <= 0x20) {
        adjust_texture_quality_for_memory();
    }
}

// ReAgent candidates for this address and its thunk (0x0063b4c0) were
// both rejected only for the unnamed per-mode capability field at
// mode+0x474 (house style for this already-unresolved record, see
// hp6::render::initialize_default_render_state and
// hp6::runtime::compute_dynamic_table_size, which read the same field's
// other bits).
void adjust_texture_quality_for_memory() {
    std::int32_t base_value;
    std::uint8_t shift_left;
    if (DAT_008663d5 == 0) {
        const std::uint32_t shift = (DAT_00cf1964 < 0x180) ? 0u : 1u;
        base_value = DAT_008928c0[DAT_008663b8] << shift;
        shift_left = (DAT_00cf1964 < 0x200) ? 0 : 1;
    } else {
        const auto mode_flags = *reinterpret_cast<std::uint32_t*>(
            static_cast<std::uintptr_t>(DAT_00cf19d0) + DAT_008663d8 * 0x584 + 0x474);
        base_value = ((mode_flags & 0x800) != 0) ? 0x40 : 0x20;
        if (DAT_00cf19bc > 1) {
            base_value = 0x80;
        }
        shift_left = static_cast<std::uint8_t>(DAT_008663b8);
    }
    const std::uint32_t shift_right = (DAT_00cf1964 > 0x40) ? 0u : 1u;
    DAT_008663dc = static_cast<std::uint32_t>(base_value << shift_left) >> shift_right;
    if (DAT_00cf1974 != 0) {
        DAT_008663dc = static_cast<std::uint32_t>(DAT_00cf1974);
    }
}

} // namespace hp6::render
