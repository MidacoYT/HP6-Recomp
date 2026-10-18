#include <cstdint>

std::uint8_t __fastcall FUN_006d6c09(std::uint32_t*);
std::int32_t __fastcall FUN_006d8864(std::uint32_t*);

// The owning class and the meaning of the word at offset +0x04
// are unresolved in the supplied evidence.
struct FUN_006d6ec3_owner {
    std::int32_t __thiscall FUN_006d6ec3(
        std::uint32_t* param_1, std::int32_t param_2);
};

std::int32_t __thiscall FUN_006d6ec3_owner::FUN_006d6ec3(
    std::uint32_t* param_1, std::int32_t param_2)
{
    static_assert(sizeof(void*) == 4);

    const auto address = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(this));
    const std::uint32_t offset16 =
        ((address + 0x0fu) & 0xfffffff0u) - address;
    const std::uint32_t offset128 =
        ((address + 0x7fu) & 0xffffff80u) - address;

    std::uint32_t total = 0;
    std::uint8_t lastType = 0;
    const std::uint8_t wantedType = FUN_006d6c09(param_1);
    bool found = false;

    auto* current = reinterpret_cast<std::uint32_t*>(this);
    if (current != nullptr) {
        do {
            std::int32_t size = 0;
            lastType = FUN_006d6c09(current);

            if (lastType != wantedType || param_2 == 1) {
                size = FUN_006d8864(current);
                if (total != 0) {
                    if (lastType == 4) {
                        total = ((((total - offset16) + 0x2fu)
                                  & 0xfffffff0u) - 0x20u) + offset16;
                    } else {
                        total = (((total - offset128) + 0x0fu)
                                 & 0xfffffff0u) + offset128;
                    }
                }
            }
            total += static_cast<std::uint32_t>(size);

            if (lastType == wantedType && !found) {
                found = true;
                if (param_2 == 3 || param_2 == 1) {
                    size = FUN_006d8864(param_1);
                    if (total != 0) {
                        if (lastType == 4) {
                            total = ((((total - offset16) + 0x2fu)
                                      & 0xfffffff0u) - 0x20u) + offset16;
                        } else {
                            total = (((total - offset128) + 0x0fu)
                                     & 0xfffffff0u) + offset128;
                        }
                    }
                    total += static_cast<std::uint32_t>(size);
                }
            }

            const std::uint32_t nextOffset = current[1]; // offset +0x04
            current = nextOffset == 0
                ? nullptr
                : reinterpret_cast<std::uint32_t*>(
                    reinterpret_cast<std::uintptr_t>(current) + nextOffset);
        } while (current != nullptr);

        if (found) {
            return static_cast<std::int32_t>(total);
        }
    }

    if (param_2 != 2) {
        if (total != 0) {
            if (lastType == 4) {
                total = ((((total - offset16) + 0x2fu)
                          & 0xfffffff0u) - 0x20u) + offset16;
            } else {
                total = (((total - offset128) + 0x0fu)
                         & 0xfffffff0u) + offset128;
            }
        }
        total += static_cast<std::uint32_t>(FUN_006d8864(param_1));
    }

    return static_cast<std::int32_t>(total);
}