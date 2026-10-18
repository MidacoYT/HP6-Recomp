#include <cstddef>
#include <cstdint>

void __cdecl FUN_006b1a2e(int param_1, std::uint64_t* param_2,
                           std::uint32_t* param_3)
{
    auto* base = reinterpret_cast<std::byte*>(param_1);
    auto* object = *reinterpret_cast<std::uint32_t**>(base + 0x10);
    auto* vtable = *reinterpret_cast<std::uint32_t**>(object);

    // The concrete types and names of these fields and the virtual method
    // are unresolved in the supplied evidence.
    using Slot24 = std::uint64_t (__cdecl*)(std::uint32_t);
    auto method = *reinterpret_cast<Slot24*>(
        reinterpret_cast<std::byte*>(vtable) + 0x24);

    *param_2 = method(*reinterpret_cast<std::uint32_t*>(base + 0x08));
    param_3[0] = *reinterpret_cast<std::uint32_t*>(base + 0x18);
    param_3[1] = *reinterpret_cast<std::uint32_t*>(base + 0x1c);
}