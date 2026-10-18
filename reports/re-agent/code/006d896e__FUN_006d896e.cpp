#include <cstdint>

// Field meanings at +0x04 and +0x0c are unresolved.
extern std::uint8_t __fastcall FUN_006d6c09(std::uint32_t*);
extern std::int32_t __cdecl FUN_006d74ce(std::int32_t);
extern std::int32_t __cdecl FUN_006d760a(std::int32_t);
extern std::uint32_t __stdcall FUN_006d7667();
extern std::int32_t __cdecl FUN_006d768d(std::int32_t);
extern std::int32_t __cdecl FUN_006d767c(std::int32_t);
extern std::int32_t __cdecl FUN_006d766b(std::int32_t);
extern std::int32_t __fastcall FUN_006d8864(std::uint32_t*);

std::uint32_t __thiscall FUN_006d896e(
    void* self, std::int32_t param_1, std::int32_t param_2)
{
    auto* const input = reinterpret_cast<std::uint8_t*>(
        static_cast<std::uint32_t>(param_1));
    std::uint32_t flags =
        *reinterpret_cast<std::uint32_t*>(input + 0x04);
    std::uint32_t size = 0;
    auto* node = static_cast<std::uint8_t*>(self);
    bool had_previous_node = false;

    for (;;) {
        if (node == nullptr) {
            if (param_2 == 3 || param_2 == 1) {
                if (flags & 0x02) {
                    const auto n = FUN_006d74ce(param_1);
                    size = static_cast<std::uint32_t>(n) - 0x20u
                         + ((size + 0x7fu) & 0xffffff80u);
                }
                if (flags & 0x04) {
                    const auto n = FUN_006d760a(param_1);
                    size = ((size + 0x0fu) & 0xfffffff0u)
                         + static_cast<std::uint32_t>(n);
                }
                if (flags & 0x100) {
                    const auto n = FUN_006d7667();
                    size = ((size + 0x0fu) & 0xfffffff0u) + n;
                }
                if (flags & 0x10) {
                    const auto n = FUN_006d768d(param_1);
                    size = ((size + 0x0fu) & 0xfffffff0u)
                         + static_cast<std::uint32_t>(n);
                }
                if ((flags & 0x80) == 0x80) {
                    const auto n = FUN_006d767c(param_1);
                    size = static_cast<std::uint32_t>(n) + 0x10u
                         + ((size - 1u) & 0xfffffff0u);
                }
                if (flags & 0x20) {
                    const auto n = FUN_006d766b(param_1);
                    size = static_cast<std::uint32_t>(n) + 0x10u
                         + ((size - 1u) & 0xfffffff0u);
                }
            }
            return size;
        }

        // Ghidra leaves the upper 24 return bits undefined. The cases
        // below use the documented byte result.
        const auto kind = FUN_006d6c09(
            reinterpret_cast<std::uint32_t*>(node));
        const auto field_0c =
            *reinterpret_cast<std::uint32_t*>(node + 0x0c);

        switch (kind) {
        case 1:
            break;

        case 2:
            if ((flags & 2) == 0 || param_2 != 2) {
                if (had_previous_node)
                    size = ((size + 0x7fu) & 0xffffff80u) - 0x20u;
                if ((flags & 2) == 0 || param_2 != 3)
                    size = size + 0x20u + field_0c;
                else
                    size += static_cast<std::uint32_t>(
                        FUN_006d74ce(param_1));
            }
            flags &= 0xfffffffdu;
            break;

        case 4:
            if ((flags & 4) == 0 || param_2 != 2) {
                if (had_previous_node)
                    size = (size + 0x0fu) & 0xfffffff0u;
                if ((flags & 4) == 0 || param_2 != 3)
                    size = size + 0x20u + field_0c;
                else
                    size += static_cast<std::uint32_t>(
                        FUN_006d760a(param_1));
            }
            flags &= 0xfffffffbu;
            break;

        case 0x10:
            if ((flags & 0x10) == 0 || param_2 != 2) {
                if (had_previous_node)
                    size = (size + 0x0fu) & 0xfffffff0u;
                if ((flags & 0x10) == 0 || param_2 != 3)
                    size = size + 0x20u + field_0c;
                else
                    size += static_cast<std::uint32_t>(
                        FUN_006d768d(param_1));
            }
            flags &= 0xffffffefu;
            break;

        case 0x20:
            if ((flags & 0x20) == 0 || param_2 != 2) {
                if (had_previous_node)
                    size = ((size - 1u) & 0xfffffff0u) + 0x10u;
                if ((flags & 0x20) == 0 || param_2 != 3)
                    size = size + 0x10u + field_0c;
                else
                    size += static_cast<std::uint32_t>(
                        FUN_006d766b(param_1));
            }
            flags &= 0xffffffdfu;
            break;

        case 0x80:
            if ((flags & 0x80) == 0 || param_2 != 2) {
                if (had_previous_node)
                    size = ((size - 1u) & 0xfffffff0u) + 0x10u;
                if ((flags & 0x80) == 0 || param_2 != 3)
                    size = size + 0x10u + field_0c;
                else
                    size += static_cast<std::uint32_t>(
                        FUN_006d767c(param_1));
            }
            flags &= 0xffffff7fu;
            break;

        case 0x100:
            // Unreachable if FUN_006d6c09 truly returns only a byte;
            // retained because the decompiled machine code tests this case.
            if ((flags & 0x100) == 0 || param_2 != 2) {
                if (had_previous_node)
                    size = (size + 0x0fu) & 0xfffffff0u;
                if ((flags & 0x100) == 0 || param_2 != 3)
                    size += static_cast<std::uint32_t>(
                        FUN_006d8864(reinterpret_cast<std::uint32_t*>(node)));
                else
                    size += FUN_006d7667();
            }
            flags &= 0xfffffeffu;
            break;

        default:
            return 0;
        }

        had_previous_node = true;
        const auto next_offset =
            *reinterpret_cast<std::uint32_t*>(node + 0x04);
        node = next_offset == 0 ? nullptr : node + next_offset;
    }
}