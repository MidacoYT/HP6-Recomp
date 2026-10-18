void __cdecl FUN_006b2408(uint32_t param_1, uint8_t param_2)
{
    int32_t iVar1 = FUN_006b18df(param_1, 1);
    if (iVar1 == 0) {
        return;
    }

    // Member names for offsets 0x28, 0x10, and 0x12 are unresolved.
    FUN_006d92a1(iVar1 + 0x28);
    uint32_t uVar2 = FUN_006b1ffc(
        reinterpret_cast<void*>(iVar1 + 0x10), param_1, 1);
    FUN_006d92ac(iVar1 + 0x28);

    if (uVar2 != 0) {
        *reinterpret_cast<uint8_t*>(uVar2 + 0x12) = param_2;
        FUN_006b2199(uVar2);
    }
}
