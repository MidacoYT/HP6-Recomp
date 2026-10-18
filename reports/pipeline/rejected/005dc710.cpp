#include <cstdint>

extern std::uint32_t* __stdcall FUN_005db920();

void __cdecl FUN_005dc710(int param_1)
{
    if (param_1 != 0) {
        const auto* slots = reinterpret_cast<const std::uint32_t*>(
            static_cast<std::uintptr_t>(*FUN_005db920()));

        // The identity and declared type of the slot at offset 0xC are unresolved.
        const auto callback =
            reinterpret_cast<void (__cdecl*)(int, int)>(slots[3]);
        callback(param_1, 0);
    }
}
