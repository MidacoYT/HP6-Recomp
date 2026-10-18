std::uint32_t __cdecl FUN_0064c300(std::int32_t param_1, std::int32_t param_2)
{
    std::uint32_t result = 0;
    auto* cachedValues = reinterpret_cast<std::int32_t*>(&DAT_00c75188);

    if (cachedValues[param_1] != param_2) {
        // The object's type and the name of its virtual method at offset 0xE4 are unresolved.
        using VirtualMethod = std::uint32_t(__thiscall*)(
            void*, std::int32_t, std::int32_t);
        auto* vtable = *reinterpret_cast<void***>(DAT_00cf1944);
        result = reinterpret_cast<VirtualMethod>(vtable[0xE4 / 4])(
            DAT_00cf1944, param_1, param_2);
        cachedValues[param_1] = param_2;
    }

    return result;
}
