#include <cstddef>
#include <cstdint>

struct FUN_006dd00c_layout {
    std::byte unknown_0x00[0x20];
    std::int32_t entry_0x20[32];
    std::uint32_t count_0xA0;
    std::int32_t field_0xA4;
    std::int32_t field_0xA8;
    std::int32_t field_0xAC;
};

static_assert(offsetof(FUN_006dd00c_layout, count_0xA0) == 0xA0);
static_assert(offsetof(FUN_006dd00c_layout, field_0xAC) == 0xAC);

std::uint32_t __thiscall FUN_006dd00c(void* this_, std::int32_t param_1)
{
    auto* object = static_cast<FUN_006dd00c_layout*>(this_);
    std::uint32_t eax; // Incoming EAX is unspecified when param_1 is zero.

    if (param_1 != 0) {
        eax = object->count_0xA0;
        std::int32_t index = 0;
        auto* entry = object->entry_0x20;

        if (static_cast<std::int32_t>(eax) > 0) {
            do {
                eax = static_cast<std::uint32_t>(*entry);
                if (*entry == param_1) {
                    eax = object->count_0xA0 - 1;
                    object->count_0xA0 = eax;
                    *entry = object->entry_0x20[eax];

                    if (object->field_0xAC == param_1)
                        object->field_0xAC = 0;
                    if (object->field_0xA8 == param_1)
                        object->field_0xA8 = 0;
                    if (object->field_0xA4 == param_1)
                        object->field_0xA4 = 0;

                    const auto address = reinterpret_cast<std::uintptr_t>(
                        &object->field_0xA4);
                    return (static_cast<std::uint32_t>(address) & 0xFFFFFF00u) | 1u;
                }

                ++index;
                ++entry;
            } while (index < static_cast<std::int32_t>(object->count_0xA0));
        }
    }

    return eax & 0xFFFFFF00u;
}