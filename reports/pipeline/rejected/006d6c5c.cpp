#include <cstdint>

extern std::uint8_t __fastcall FUN_006d6c09(std::uint32_t*);

std::uint32_t* __thiscall FUN_006d6c5c(void* this_, std::int32_t param_1)
{
    auto* node = static_cast<std::uint8_t*>(this_);

    while (node != nullptr) {
        if (param_1 == static_cast<std::int32_t>(
                           FUN_006d6c09(reinterpret_cast<std::uint32_t*>(node)))) {
            return reinterpret_cast<std::uint32_t*>(node);
        }

        // The member at +0x04 is a relative byte offset; its name is unresolved.
        const std::uint32_t offset_04 =
            *reinterpret_cast<std::uint32_t*>(node + 0x04);

        node = offset_04 == 0 ? nullptr : node + offset_04;
    }

    return nullptr;
}
