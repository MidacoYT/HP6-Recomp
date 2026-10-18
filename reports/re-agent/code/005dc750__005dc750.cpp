void __cdecl FUN_005dc750(std::uint32_t param_1, std::uint32_t param_2)
{
    // The P-code branches past initialization when bit 0 is clear.
    if ((_DAT_00fd1254 & 1u) != 0) {
        _DAT_00fd1254 |= 1u;
        DAT_00fcdd74 = &PTR_FUN_0083e55c;
        DAT_00fcdd78 = &PTR_FUN_0083e56c;
        _DAT_00cef6bc = &DAT_00fcdd74;
        FUN_006b936d((_onexit_t)&LAB_00711e60);
    }

    std::uint32_t arguments[3];
    arguments[1] = param_2;
    arguments[0] = 1u;
    arguments[2] = 0u;

    using Function = void (__cdecl *)(std::uint32_t, void*);
    reinterpret_cast<Function>(*DAT_00fcdd78)(param_1, arguments);
}