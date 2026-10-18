// The available evidence does not identify member names for offsets
// 0x20, 0xa0, or 0xb0.
extern std::uint32_t PTR_LAB_0071b520;
extern std::uint32_t PTR_LAB_0071b540;

void __fastcall FUN_006dcf14(std::uint32_t* param_1)
{
    auto* puVar1 = reinterpret_cast<std::uint32_t*>(param_1[0x2c]);
    param_1[0] = reinterpret_cast<std::uint32_t>(&PTR_LAB_0071b540);

    while (true) {
        if (puVar1 != nullptr) {
            auto* vtable = reinterpret_cast<std::uint32_t*>(*puVar1);
            auto call = reinterpret_cast<void (__thiscall*)(std::uint32_t*, std::uint32_t)>(vtable[0]);
            call(puVar1, 1);
        }

        if (static_cast<std::int32_t>(param_1[0x28]) < 1) {
            break;
        }
        puVar1 = reinterpret_cast<std::uint32_t*>(param_1[8]);
    }

    param_1[0x28] = 0;
    param_1[0] = reinterpret_cast<std::uint32_t>(&PTR_LAB_0071b520);
}