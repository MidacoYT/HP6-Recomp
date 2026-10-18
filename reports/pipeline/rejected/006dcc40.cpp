// The member types and class name are unresolved. These offset names preserve
// the 32-bit layout shown by the decompile.
void __fastcall FUN_006dcc40(std::uint32_t* param_1)
{
    auto* array_0xb8 =
        reinterpret_cast<std::uint32_t*>(param_1[0x2e]);

    param_1[0] = 0x0071b500;

    if (array_0xb8 != nullptr) {
        if (array_0xb8[-1] == 0) {
            FUN_005dc710(reinterpret_cast<int>(array_0xb8 - 1));
        } else {
            auto* vtable = reinterpret_cast<std::uint32_t*>(array_0xb8[0]);
            reinterpret_cast<void (*)(int)>(vtable[0])(3);
        }
    }

    auto* direct_input_0xb4 =
        reinterpret_cast<std::uint32_t*>(param_1[0x2d]);

    if (direct_input_0xb4 != nullptr) {
        auto* vtable = reinterpret_cast<std::uint32_t*>(direct_input_0xb4[0]);
        reinterpret_cast<void (__stdcall*)(void*)>(vtable[2])(
            direct_input_0xb4);
        param_1[0x2d] = 0;
    }

    FUN_006dcf14(param_1);
}
