// The types of the table entries and cleanup callback are unresolved.
// These globals and symbols retain their address-based names.
std::uint32_t* __stdcall FUN_005db920()
{
    if ((_DAT_00fd1254 & 1u) == 0) {
        _DAT_00fd1254 |= 1u;
        DAT_00fcdd74 = &PTR_FUN_0083e55c;
        DAT_00fcdd78 = &PTR_FUN_0083e56c;
        _DAT_00cef6bc = &DAT_00fcdd74;
        FUN_006b936d((_onexit_t)&LAB_00711e60);
    }

    return &DAT_00fcdd74;
}
