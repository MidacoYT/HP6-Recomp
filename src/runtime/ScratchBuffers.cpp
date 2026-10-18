#include "runtime/ScratchBuffers.h"

#include <cstring>

// Addresses read/written by the binary but absent from the exported
// globals; the final binary must supply their stored values explicitly.
extern void* DAT_00fcade0;
extern std::int32_t* DAT_00cf18e4;         // bump arena: [0]=base,[1]=capacity,[2]=current
extern volatile std::uint32_t _DAT_00000000; // fatal-trap target (deliberate null write)
extern std::byte _Src_00c754d0[];
extern std::int32_t DAT_00cf19a0;
extern std::uint32_t* DAT_00cf1a08;        // pointer to a cursor variable
extern std::uint32_t* DAT_00cf1920;        // last bump-allocated record
extern std::int32_t DAT_00866368;          // plain 32-bit value; also read as a
                                           // %d in hp6::render::report_mode_option,
                                           // so this is not a vtable/function pointer
extern std::uint32_t DAT_00cf1918;
extern std::uint32_t DAT_00cf191c;
extern std::byte _Dst_00bb0a70[];
extern std::byte _Dst_0096b490[];

namespace hp6::runtime {

void store_payload(void* self, const void* data, std::size_t size) {
    auto* bytes = static_cast<std::uint8_t*>(self);
    void* destination;
    if (static_cast<std::int32_t>(size) < 0x800) {
        destination = bytes + 0x10;
    } else {
        destination = *reinterpret_cast<void**>(
            static_cast<std::uint8_t*>(DAT_00fcade0) + 0x18);
        if (destination == nullptr) {
            return;
        }
        if (static_cast<std::int32_t>(size) > 0x23ff) {
            return;
        }
    }
    *reinterpret_cast<void**>(bytes + 0x810) = destination;
    std::memcpy(destination, data, size);
    bytes[0x8] = 1;
}

std::uint32_t bump_allocate(std::uint32_t /*unused_param_1*/, std::uint32_t size) {
    std::int32_t* arena = DAT_00cf18e4;
    const auto limit = static_cast<std::uint32_t>(arena[0] + arena[1]);
    const auto current = static_cast<std::uint32_t>(arena[2]);
    const auto aligned = (size + 3u) & ~3u;
    const auto advanced = aligned + current;
    arena[2] = static_cast<std::int32_t>(advanced);
    if (advanced >= limit) {
        _DAT_00000000 = 0;
    }
    return current;
}

std::int32_t advance_cursor_copy(std::uint8_t* frame, std::int32_t count,
                                 std::uint32_t /*unused_param_3*/,
                                 void* destination) {
    auto* cursor = reinterpret_cast<std::uint8_t**>(frame + 0x1c);
    const auto byte_count = static_cast<std::size_t>(count) << 2;
    std::memcpy(destination, *cursor, byte_count);
    *cursor += byte_count;
    return count;
}

std::int32_t copy_fixed_records(void* destination) {
    const auto count = DAT_00cf19a0;
    std::memcpy(destination, _Src_00c754d0,
               static_cast<std::size_t>(count) << 3);
    return count;
}

void append_to_cursor(const void* source, std::int32_t count) {
    if (*DAT_00cf1a08 != 0) {
        const auto byte_count = static_cast<std::uint32_t>(count) << 1;
        std::memcpy(reinterpret_cast<void*>(
                       static_cast<std::uintptr_t>(*DAT_00cf1a08)),
                   source, byte_count);
        *DAT_00cf1a08 += byte_count;
    }
}

std::uint32_t init_payload_record(void* self, const std::uint32_t* source,
                                  char copy_a, char copy_b) {
    DAT_00cf1920 = reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(
        bump_allocate(static_cast<std::uint32_t>(
                         reinterpret_cast<std::uintptr_t>(self)),
                     0x30)));
    for (int i = 0; i != 12; ++i) {
        DAT_00cf1920[i] = source[i];
    }

    const auto* source_bytes = reinterpret_cast<const std::uint8_t*>(source);
    if (copy_a != 0) {
        DAT_00cf1920[4] = static_cast<std::uint32_t>(DAT_00866368);
        const auto count = *(source_bytes + 0x24);
        const auto stride = *reinterpret_cast<const std::uint16_t*>(source_bytes + 0x26);
        DAT_00cf1918 = static_cast<std::uint32_t>(count) * stride;
        const auto src = *reinterpret_cast<const std::uint32_t*>(source_bytes + 0x28);
        std::memcpy(_Dst_00bb0a70,
                   reinterpret_cast<const void*>(static_cast<std::uintptr_t>(src)),
                   DAT_00cf1918);
    }
    if (copy_b != 0) {
        DAT_00cf1920[5] = static_cast<std::uint32_t>(DAT_00866368);
        const auto stride = *reinterpret_cast<const std::uint16_t*>(source_bytes + 0x22);
        DAT_00cf191c = static_cast<std::uint32_t>(stride) << 1;
        const auto src = *reinterpret_cast<const std::uint32_t*>(source_bytes + 0x2c);
        std::memcpy(_Dst_0096b490,
                   reinterpret_cast<const void*>(static_cast<std::uintptr_t>(src)),
                   DAT_00cf191c);
    }
    return 1u;
}

} // namespace hp6::runtime
