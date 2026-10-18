#include <cstddef>
#include <cstdint>
#include <cstring>

// The type of DAT_00fcade0 and the names of the fields at these offsets
// are unresolved in the supplied evidence.
extern void* DAT_00fcade0;

void __thiscall FUN_00422e20(void* self, void* param_1, std::size_t param_2)
{
    static_assert(sizeof(std::size_t) == 4);

    auto* bytes = static_cast<std::byte*>(self);
    void* destination;

    if (static_cast<std::int32_t>(param_2) < 0x800) {
        destination = bytes + 0x10;
    } else {
        destination = *reinterpret_cast<void**>(
            static_cast<std::byte*>(DAT_00fcade0) + 0x18);
        if (destination == nullptr) {
            return;
        }
        if (static_cast<std::int32_t>(param_2) > 0x23ff) {
            return;
        }
    }

    *reinterpret_cast<void**>(bytes + 0x810) = destination;
    ::memcpy(destination, param_1, param_2);
    *reinterpret_cast<std::uint8_t*>(bytes + 0x8) = 1;
}
