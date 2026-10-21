#include "render/GeometryBatcher.h"

#include <cstdint>
#include <cstring>

// 0x006512f0, 0x00650f70, 0x00651d60 and 0x00651f80. These globals
// share their original x86 layout with RenderStateCache.cpp.
extern void* DAT_00cf1944;
extern void* DAT_00cf19f0;
extern void* DAT_00cf19f4;
extern void* DAT_00cf19f8;
extern void* DAT_00cf19fc;
extern void** DAT_00cf1a00;
extern void* _DAT_00cf1a04;
extern std::uint32_t* DAT_00cf1a08;
extern void* _DAT_00cf1a0c;
extern std::uint32_t DAT_00cf1a18;
extern std::uint32_t DAT_00cf1a1c;
extern void* DAT_00cf1a20;
extern std::uint32_t DAT_00cf1a24;
extern std::uint32_t DAT_00cf19de;
extern std::uint8_t DAT_008663d5;
extern std::uint32_t DAT_008663e4;
extern std::uint32_t DAT_008663e8;
extern std::uint32_t DAT_008663ec;
extern std::uint32_t DAT_00cf1a28;
extern std::uint32_t DAT_00cf1a2c;
extern std::uint32_t DAT_008663f0[];
extern std::uint8_t DAT_0098b490[];
extern std::uint8_t DAT_00bb09f8[];
extern std::uint8_t DAT_00a70890[];
extern std::uint8_t DAT_00baa170[];
extern std::uint8_t DAT_0098c6a8[];
extern std::uint8_t DAT_009985a0[];

namespace hp6::render {
namespace {
constexpr std::uint32_t invalid_handle = 0xbad1bad1u;
constexpr std::uint32_t dynamic_bit = 0x80000000u;

struct PoolEntry {
    std::uint32_t capacity;
    std::uint8_t* cursor;
    std::uint8_t* base;
    std::uint32_t locked;
    void* buffer;
};
#if INTPTR_MAX == INT32_MAX
static_assert(sizeof(PoolEntry) == 0x14, "x86 D3D buffer pool entry");
#endif

std::uint32_t cursor_span(const std::uint8_t* cursor, const std::uint8_t* base) {
    return static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(cursor) -
        reinterpret_cast<std::uintptr_t>(base));
}

using CreateVertexBuffer = long(__stdcall*)(void*, unsigned, unsigned,
                                            unsigned, unsigned, void**, void*);
using CreateIndexBuffer = long(__stdcall*)(void*, unsigned, unsigned,
                                           unsigned, unsigned, void**, void*);
using LockBuffer = long(__stdcall*)(void*, unsigned, unsigned, void**, unsigned);
using UnlockBuffer = long(__stdcall*)(void*);

template <typename Function> Function method(void* object, std::size_t offset) {
    return *reinterpret_cast<Function const*>(
        static_cast<const std::uint8_t*>(*static_cast<void**>(object)) + offset);
}

PoolEntry* vertex_entry(std::uint32_t handle) {
    auto* table = (handle & dynamic_bit) ? DAT_00baa170 : DAT_00a70890;
    return reinterpret_cast<PoolEntry*>(table + (handle & ~dynamic_bit) * 0x14u);
}
PoolEntry* index_entry(std::uint32_t handle) {
    auto* table = (handle & dynamic_bit) ? DAT_009985a0 : DAT_0098c6a8;
    return reinterpret_cast<PoolEntry*>(table + (handle & ~dynamic_bit) * 0x14u);
}

std::uint32_t acquire_vertex_buffer(std::uint32_t count, std::uint32_t stride,
                                    std::uint32_t mode) {
    // The low-tier 0x24 stride is rejected by the original pool selector.
    if (DAT_008663d5 != 0 && stride == 0x24) {
        return invalid_handle;
    }
    const std::uint32_t group = DAT_008663f0[(stride - 0xc) >> 2];
    auto* counters = mode == 0 ? DAT_0098b490 + DAT_008663e4 * 0x48
                               : DAT_00bb09f8;
    auto* count_slot = reinterpret_cast<std::uint32_t*>(counters + group * 4 + 4);
    const std::uint32_t last = *count_slot == 0 ? 0 : *count_slot - 1;
    std::uint32_t slot = (group << 5) | last |
                         (mode == 0 ? DAT_008663e4 * 0x200 : 0);
    PoolEntry* entry = vertex_entry(slot | (mode ? dynamic_bit : 0));
    const auto used = cursor_span(entry->cursor, entry->base);
    if (entry->buffer == nullptr || entry->capacity < count * stride + used ||
        (stride == 0x20 && mode != 0 && slot == 0x60)) {
        if (*count_slot != 0) {
            ++slot;
        }
        entry = vertex_entry(slot | (mode ? dynamic_bit : 0));
        if (entry->buffer == nullptr) {
            const auto usage = mode == 0 ? DAT_00cf1a28 : DAT_008663ec;
            const auto pool = mode == 0 ? DAT_008663e8 : DAT_00cf1a2c;
            method<CreateVertexBuffer>(DAT_00cf1944, 0x68)(
                DAT_00cf1944, stride << 13, usage, 0, pool,
                &entry->buffer, nullptr);
        }
        entry->cursor = entry->base = nullptr;
        entry->capacity = stride << 13;
        ++*count_slot;
    }
    return slot | (mode ? dynamic_bit : 0);
}

std::uint32_t acquire_index_buffer(std::uint32_t bytes, std::uint32_t mode) {
    auto* counters = mode == 0 ? DAT_0098b490 + DAT_008663e4 * 0x48
                               : DAT_00bb09f8;
    auto* count_slot = reinterpret_cast<std::uint32_t*>(counters + 0x44);
    const std::uint32_t last = *count_slot == 0 ? 0 : *count_slot - 1;
    std::uint32_t slot = (mode == 0 ? DAT_008663e4 << 5 : 0) | last;
    PoolEntry* entry = index_entry(slot | (mode ? dynamic_bit : 0));
    const auto used = cursor_span(entry->cursor, entry->base);
    if (entry->buffer == nullptr || entry->capacity < bytes + used ||
        (mode != 0 && slot == 0)) {
        if (*count_slot != 0) {
            ++slot;
        }
        entry = index_entry(slot | (mode ? dynamic_bit : 0));
        if (entry->buffer == nullptr) {
            const auto usage = mode == 0 ? DAT_00cf1a28 : DAT_008663ec;
            const auto pool = mode == 0 ? DAT_008663e8 : DAT_00cf1a2c;
            method<CreateIndexBuffer>(DAT_00cf1944, 0x6c)(
                DAT_00cf1944, 0x20000, usage, 0x65, pool,
                &entry->buffer, nullptr);
        }
        entry->cursor = entry->base = nullptr;
        entry->capacity = 0x20000;
        ++*count_slot;
    }
    return slot | (mode ? dynamic_bit : 0);
}

void unlock_if_locked(PoolEntry* entry) {
    if (entry->locked != 0) {
        method<UnlockBuffer>(entry->buffer, 0x30)(entry->buffer);
        entry->locked = 0;
    }
}

void lock_scratch_buffers(BatchCommand* command) {
    const std::uint32_t vertex_bytes = command->vertex_count_26 * DAT_00cf1a18;
    const auto vertex_handle = acquire_vertex_buffer(
        command->vertex_count_26, DAT_00cf1a18, DAT_00cf1a1c);
    *reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uint8_t*>(command) + 0x10) =
        vertex_handle;
    if (vertex_handle == invalid_handle) {
        *reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uint8_t*>(command) + 0x14) =
            invalid_handle;
        return;
    }
    PoolEntry* vertex = vertex_entry(vertex_handle);
    const auto vertex_offset = cursor_span(vertex->cursor, vertex->base);
    *reinterpret_cast<std::uint16_t*>(reinterpret_cast<std::uint8_t*>(command) + 0x20) =
        static_cast<std::uint16_t>(vertex_offset / DAT_00cf1a18);
    const auto flags = DAT_00cf1a1c == 0 ? 0x800u : 0x2000u;
    if (vertex->locked == 0) {
        method<LockBuffer>(vertex->buffer, 0x2c)(vertex->buffer, vertex_offset,
            DAT_00cf1a1c == 0 ? vertex_bytes : 0, reinterpret_cast<void**>(&vertex->cursor), flags);
        vertex->locked = 1;
    }
    if (vertex_offset == 0) {
        vertex->base = vertex->cursor;
    }
    DAT_00cf1a00 = reinterpret_cast<void**>(&vertex->cursor);
    _DAT_00cf1a04 = vertex->base;

    if ((DAT_00cf1a24 & 1) == 0) {
        const auto index_handle = acquire_index_buffer(
            command->index_count_22 * 2u, DAT_00cf1a1c);
        *reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uint8_t*>(command) + 0x14) =
            index_handle;
        PoolEntry* index = index_entry(index_handle);
        const auto index_offset = cursor_span(index->cursor, index->base);
        command->word_1c = static_cast<std::uint16_t>(index_offset >> 1);
        if (index->locked == 0) {
            method<LockBuffer>(index->buffer, 0x2c)(index->buffer, index_offset,
                DAT_00cf1a1c == 0 ? command->index_count_22 * 2u : 0,
                reinterpret_cast<void**>(&index->cursor), flags);
            index->locked = 1;
        }
        index->base = index_offset == 0 ? index->cursor : index->cursor - index_offset;
        DAT_00cf1a08 = reinterpret_cast<std::uint32_t*>(&index->cursor);
        _DAT_00cf1a0c = index->base;
    } else {
        *reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uint8_t*>(command) + 0x14) =
            invalid_handle;
    }
}
} // namespace

void flush_vertex_batch() {
    auto* command = static_cast<BatchCommand*>(DAT_00cf1a20);
    if (command->vertex_count_26 == 0) {
        command->vertex_count_26 = static_cast<std::uint16_t>(
            (reinterpret_cast<std::uintptr_t>(DAT_00cf19f8) -
             reinterpret_cast<std::uintptr_t>(DAT_00cf19f0)) / DAT_00cf1a18);
        if (command->vertex_count_26 == 0) {
            *reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uint8_t*>(command) + 0x10) = invalid_handle;
            *reinterpret_cast<std::uint32_t*>(reinterpret_cast<std::uint8_t*>(command) + 0x14) = invalid_handle;
        } else {
            command->index_count_22 = static_cast<std::uint16_t>(
                (reinterpret_cast<std::uintptr_t>(DAT_00cf19fc) -
                 reinterpret_cast<std::uintptr_t>(DAT_00cf19f4)) >> 1);
            lock_scratch_buffers(command);
            const auto vertex_bytes = command->vertex_count_26 * DAT_00cf1a18;
            if (*DAT_00cf1a00 != nullptr) {
                std::memcpy(*DAT_00cf1a00, DAT_00cf19f0, vertex_bytes);
            }
            const auto vertex_handle = *reinterpret_cast<std::uint32_t*>(
                reinterpret_cast<std::uint8_t*>(command) + 0x10);
            if ((vertex_handle & dynamic_bit) == 0) {
                unlock_if_locked(vertex_entry(vertex_handle));
            }
            if (*DAT_00cf1a00 != nullptr) {
                *DAT_00cf1a00 = static_cast<std::uint8_t*>(*DAT_00cf1a00) + vertex_bytes;
            }
            if ((DAT_00cf1a24 & 1) == 0) {
                const auto index_bytes = command->index_count_22 * 2u;
                if (*DAT_00cf1a08 != 0) {
                    std::memcpy(reinterpret_cast<void*>(static_cast<std::uintptr_t>(*DAT_00cf1a08)),
                                DAT_00cf19f4, index_bytes);
                }
                const auto index_handle = *reinterpret_cast<std::uint32_t*>(
                    reinterpret_cast<std::uint8_t*>(command) + 0x14);
                if ((index_handle & dynamic_bit) == 0) {
                    unlock_if_locked(index_entry(index_handle));
                }
                if (*DAT_00cf1a08 != 0) {
                    *DAT_00cf1a08 += index_bytes;
                }
            }
        }
    } else {
        const auto vertex_handle = *reinterpret_cast<std::uint32_t*>(
            reinterpret_cast<std::uint8_t*>(command) + 0x10);
        if (vertex_handle != invalid_handle && (vertex_handle & dynamic_bit) == 0) {
            unlock_if_locked(vertex_entry(vertex_handle));
        }
        const auto index_handle = *reinterpret_cast<std::uint32_t*>(
            reinterpret_cast<std::uint8_t*>(command) + 0x14);
        if (index_handle != invalid_handle && (index_handle & dynamic_bit) == 0) {
            unlock_if_locked(index_entry(index_handle));
        }
    }
    DAT_00cf19de = 0;
    _DAT_00cf1a04 = nullptr;
    _DAT_00cf1a0c = nullptr;
}

} // namespace hp6::render
