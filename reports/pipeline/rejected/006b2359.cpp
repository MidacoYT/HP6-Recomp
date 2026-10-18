#include <cstdint>

extern std::int32_t __cdecl FUN_006b18df(std::uint32_t, char);
extern void __fastcall FUN_006d92a1(std::int32_t);
extern std::uint32_t __thiscall FUN_006b1ffc(void*, std::int32_t, char);
extern void __fastcall FUN_006d92ac(std::int32_t);

// The 8-byte return type and the two indirect-call signatures remain unresolved.
std::uint64_t __cdecl FUN_006b2359(std::uint32_t param_1)
{
    const std::int32_t base = FUN_006b18df(param_1, 1);

    FUN_006d92a1(base + 0x28);
    auto* entry = reinterpret_cast<std::uint32_t*>(
        FUN_006b1ffc(
            reinterpret_cast<void*>(static_cast<std::uintptr_t>(base + 0x1c)),
            static_cast<std::int32_t>(param_1), 1));
    FUN_006d92ac(base + 0x28);

    const auto vtable = static_cast<std::uintptr_t>(*entry);
    const auto get_value = *reinterpret_cast<std::uint64_t (__cdecl**)()>(vtable + 8);
    const std::uint64_t result = get_value();

    const auto release = *reinterpret_cast<void (__cdecl**)(int)>(vtable);
    release(1);
    return result;
}
