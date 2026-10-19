#include "render/SubmitRenderCommand.h"

#include <cstddef>
#include <cstring>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern std::uint8_t* DAT_009970d8[]; // per-bucket region-descriptor pointer table
extern std::uint8_t DAT_009978e0[];  // per-(region-descriptor's own +0x22 word) byte table
extern std::uint8_t DAT_0098c690[];  // per-bucket "touched by which region(s)" mask byte table
extern char DAT_0089cbfe;            // active region's own mask byte (hp6::render::RenderRegionManager.cpp)
extern std::uint8_t* DAT_00967218;   // active region's own 0x10000-byte block (same file)
extern std::int32_t DAT_00998580;    // active render-region's own mode (hp6::render::RenderStateCache.cpp)
extern void* DAT_0096b48c;           // shared bump write-pointer for queued commands
extern void* _Dst_00b10ce8[];        // global per-material-subtype bucket tails ("indexed" mode)
extern void* _Dst_00966fd8[];        // global per-material-subtype bucket heads ("indexed" mode)

namespace hp6::render {
namespace {

std::uint32_t region_mode(std::int16_t bucket) {
    return *reinterpret_cast<std::uint32_t*>(DAT_009970d8[bucket] + 0x14);
}

} // namespace

void submit_render_command(std::int16_t bucket, std::int32_t* command) {
    std::uint8_t* region_descriptor = DAT_009970d8[bucket];
    const auto field_22 = *reinterpret_cast<std::uint16_t*>(region_descriptor + 0x22);
    DAT_009978e0[field_22] = 0x3f;

    const std::uint32_t mode = region_mode(bucket);
    const bool indexed_mode = (mode & 0xc0000000u) == 0x40000000u;

    if (indexed_mode) {
        DAT_0098c690[bucket] = static_cast<std::uint8_t>(DAT_0098c690[bucket] | DAT_0089cbfe);
        const std::uint32_t slot = (mode >> 10) & 0xffu;
        if (_Dst_00b10ce8[slot] == nullptr) {
            _Dst_00966fd8[slot] = DAT_0096b48c;
        } else {
            *reinterpret_cast<void**>(static_cast<std::uint8_t*>(_Dst_00b10ce8[slot]) + 0xc) =
                DAT_0096b48c;
        }
    } else {
        auto** tail = reinterpret_cast<void**>(DAT_00967218 + static_cast<std::ptrdiff_t>(bucket) * 4);
        if (*tail == nullptr) {
            DAT_0098c690[bucket] = static_cast<std::uint8_t>(DAT_0098c690[bucket] | DAT_0089cbfe);
            *reinterpret_cast<void**>(DAT_00967218 + 0x8000 + static_cast<std::ptrdiff_t>(bucket) * 4) =
                DAT_0096b48c;
        } else {
            *reinterpret_cast<void**>(static_cast<std::uint8_t*>(*tail) + 0xc) = DAT_0096b48c;
        }
    }

    auto* item = static_cast<std::uint8_t*>(DAT_0096b48c);

    // NOT YET INTEGRATED: thunk_FUN_010026a0 -- see SubmitRenderCommand.h.
    // Left as a documented deferred no-op: command[1]'s bit 0 is never
    // cleared here, unlike the exported x86's own conditional clear.

    std::size_t item_size;
    if ((static_cast<std::uint32_t>(command[1]) & 0xbfff7bcfu) == 0u) {
        std::memcpy(item, command, 0x10);
        item_size = 0x10;
    } else {
        std::memcpy(item, command, 0x24);
        item_size = 0x24;
    }

    const std::uint32_t mode_after = region_mode(bucket);
    if ((mode_after & 0xc0000000u) == 0x40000000u) {
        *reinterpret_cast<std::int16_t*>(item + 0x24) = bucket;
        *reinterpret_cast<std::int16_t*>(item + 0x26) =
            static_cast<std::int16_t>(static_cast<std::uint32_t>(command[1]) >> 0xf);
        *reinterpret_cast<std::int32_t*>(item + 0x28) = DAT_00998580;
        const std::uint32_t slot = (mode_after >> 10) & 0xffu;
        _Dst_00b10ce8[slot] = item;
        item += 0x2c;
    } else {
        *reinterpret_cast<void**>(DAT_00967218 + static_cast<std::ptrdiff_t>(bucket) * 4) = item;
        item += item_size;
    }
    DAT_0096b48c = item;
}

} // namespace hp6::render
