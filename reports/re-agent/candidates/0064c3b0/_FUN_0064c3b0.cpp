std::uint32_t __cdecl FUN_0064c3b0(
    std::int32_t param_1,
    std::int32_t param_2,
    std::int32_t param_3)
{
    std::uint32_t result = 0;
    auto* cachedValues = reinterpret_cast<std::int32_t*>(&DAT_009968d8);
    auto& cachedValue = cachedValues[param_1 * 0x20 + param_2];

    if (cachedValue != param_3) {
        // The object's type and the virtual method at offset 0x10C are unresolved.
        using VirtualMethod = std::uint32_t(__thiscall*)(
            void*, std::int32_t, std::int32_t, std::int32_t);
        auto* vtable = *reinterpret_cast<void***>(DAT_00cf1944);
        result = reinterpret_cast<VirtualMethod>(vtable[0x10C / 4])(
            DAT_00cf1944, param_1, param_2, param_3);
        cachedValue = param_3;
    }

    return result;
}
