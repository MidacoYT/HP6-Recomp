#include <cstdint>
#include <cstring>

extern void* _Src_00c754d0;
extern std::int32_t DAT_00cf19a0;

int __cdecl FUN_01003620(void* param_1)
{
    const std::uint32_t byte_count =
        static_cast<std::uint32_t>(DAT_00cf19a0) << 3;

    std::memcpy(param_1, &_Src_00c754d0, byte_count);
    return DAT_00cf19a0;
}
