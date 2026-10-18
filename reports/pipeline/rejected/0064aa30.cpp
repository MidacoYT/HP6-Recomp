void __stdcall FUN_0064aa30()
{
    using VirtualCall = uint32_t (__thiscall *)(void*);

    const auto vtable = *reinterpret_cast<uintptr_t const*>(DAT_00cf1944);
    const auto call = *reinterpret_cast<VirtualCall const*>(vtable + 0x10);

    DAT_00cf1964 = call(DAT_00cf1944) >> 20;
    if (DAT_00cf1964 != 0 && DAT_00cf1964 < 0x21) {
        thunk_FUN_01011a50();
    }
}
