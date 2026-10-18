#include <cstdint>
#include <cstring>

extern std::uint32_t* DAT_00cf1a08;

void __cdecl FUN_01003ca0(void* param_1, std::int32_t param_2)
{
    if (*DAT_00cf1a08 != 0) {
        std::memcpy(
            reinterpret_cast<void*>(static_cast<std::uintptr_t>(*DAT_00cf1a08)),
            param_1,
            static_cast<std::uint32_t>(param_2) << 1);

        *DAT_00cf1a08 =
            *DAT_00cf1a08 + static_cast<std::uint32_t>(param_2) * 2U;
    }
}
