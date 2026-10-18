#include <cstdint>

extern std::int32_t __cdecl FUN_006d74ce(std::int32_t);
extern std::int32_t __cdecl FUN_006d760a(std::int32_t);
extern std::int32_t __cdecl FUN_006d768d(std::int32_t);

std::uint32_t __cdecl FUN_006d76a6(std::int32_t param_1)
{
    const auto* base = reinterpret_cast<const std::uint8_t*>(
        static_cast<std::uintptr_t>(static_cast<std::uint32_t>(param_1)));

    auto load_u32 = [base](std::uint32_t offset) {
        return *reinterpret_cast<const std::uint32_t*>(base + offset);
    };

    std::uint32_t size = 0;
    if ((base[4] & 0x02) != 0) {
        size = static_cast<std::uint32_t>(FUN_006d74ce(param_1));
    }

    const std::uint32_t flags = load_u32(4);

    if ((flags & 0x04) != 0) {
        if (size != 0 && load_u32(0x60) == 0) {
            size = (size + 0x0f) & 0xfffffff0u;
        }
        size += static_cast<std::uint32_t>(FUN_006d760a(param_1));
    }

    if ((flags & 0x100) != 0) {
        std::uint32_t aligned = 0;
        if (size != 0) {
            aligned = (size + 0x0f) & 0xfffffff0u;
        }
        size = aligned + 0x20;
    }

    if ((flags & 0x10) != 0) {
        std::uint32_t aligned = 0;
        if (size != 0) {
            aligned = (size + 0x0f) & 0xfffffff0u;
        }
        size = aligned + static_cast<std::uint32_t>(FUN_006d768d(param_1));
    }

    if ((flags & 0x80) != 0) {
        std::uint32_t aligned = 0;
        if (size != 0) {
            aligned = ((size - 1) & 0xfffffff0u) + 0x10;
        }
        size = aligned + 0x10 + load_u32(0x84);
    }

    if ((flags & 0x20) != 0) {
        std::uint32_t aligned = 0;
        if (size != 0) {
            aligned = ((size - 1) & 0xfffffff0u) + 0x10;
        }
        size = aligned + 0x11 + load_u32(0x90);
    }

    return size;
}
// Member names for offsets 0x04, 0x60, 0x84, and 0x90 are unresolved by the supplied evidence.