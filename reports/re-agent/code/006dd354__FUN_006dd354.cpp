void __fastcall FUN_006dd354(std::uint32_t* param_1)
{
    *param_1 = reinterpret_cast<std::uint32_t>(&PTR_FUN_0071b580);

    FUN_006de5b0(reinterpret_cast<int>(param_1));

    // Member at offset 0x464; its name is unresolved.
    FUN_006dd00c(
        reinterpret_cast<void*>(param_1[0x119]),
        reinterpret_cast<int>(param_1));

    FUN_006de5e1(param_1);
}