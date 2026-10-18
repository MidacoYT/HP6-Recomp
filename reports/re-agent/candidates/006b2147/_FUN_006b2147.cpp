#include <cstddef>
#include <cstdint>

// Member names and concrete object types are unresolved in the supplied evidence.
std::uint32_t __thiscall FUN_006b1ffc(
    void* this_, std::int32_t param_1, char param_2);

void __thiscall FUN_006b2147(
    void* this_, std::int32_t param_1, std::uint32_t* param_2)
{
    auto* bytes = static_cast<std::byte*>(this_);

    *param_2 = 0;

    if (*reinterpret_cast<std::int32_t*>(bytes + 0x04) != 0) {
        auto* current = *reinterpret_cast<std::byte**>(bytes + 0x0c);

        if (current == nullptr ||
            *reinterpret_cast<std::int32_t*>(current + 0x0c) != param_1) {
            if (FUN_006b1ffc(bytes + 0x10, param_1, '\0') == 0) {
                FUN_006b1ffc(bytes + 0x1c, param_1, '\0');
            } else {
                *param_2 = 1;
            }
        } else {
            *param_2 = 0xffffffffu;
        }
    }
}
