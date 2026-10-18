#include <cstdint>

void __thiscall FUN_006d7a48(void* self, std::int32_t param_1);
void __thiscall FUN_006d7391(
    void* self,
    std::uint32_t* param_1,
    std::uint32_t* param_2,
    std::uint32_t* param_3,
    std::uint32_t* param_4);

void __thiscall FUN_006d7b49(void* self, std::int32_t param_1)
{
    FUN_006d7a48(self, param_1);

    auto* data = reinterpret_cast<std::uint8_t*>(
        static_cast<std::uintptr_t>(static_cast<std::uint32_t>(param_1)));
    data[5] = static_cast<std::uint8_t>(data[5] | 1);

    FUN_006d7391(
        self,
        reinterpret_cast<std::uint32_t*>(data + 0x64),
        reinterpret_cast<std::uint32_t*>(data + 0x68),
        reinterpret_cast<std::uint32_t*>(data + 0x6c),
        reinterpret_cast<std::uint32_t*>(data + 0x70));
}
