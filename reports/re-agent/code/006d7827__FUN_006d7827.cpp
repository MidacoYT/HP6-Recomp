void __thiscall FUN_006d7827(void* self, int param_1)
{
    // Member names and field types are unresolved; offsets preserve the observed layout.
    auto* source = reinterpret_cast<const unsigned char*>(param_1);
    auto* destination = static_cast<unsigned char*>(self);

    FUN_006d7745(self, param_1);
    *destination = 0x7a;

    const auto value_70 = *reinterpret_cast<const unsigned int*>(source + 0x70);
    const auto value_6c = *reinterpret_cast<const unsigned int*>(source + 0x6c);
    const auto value_68 = *reinterpret_cast<const unsigned int*>(source + 0x68);
    const auto value_64 = *reinterpret_cast<const unsigned int*>(source + 0x64);

    *reinterpret_cast<unsigned int*>(destination + 0x1c) = value_70;
    *reinterpret_cast<unsigned int*>(destination + 0x18) = value_6c;
    *reinterpret_cast<unsigned int*>(destination + 0x10) = value_64;
    *reinterpret_cast<unsigned int*>(destination + 0x14) = value_68;

    FUN_00688550(self, reinterpret_cast<int>(destination + 0x20), 0);
}