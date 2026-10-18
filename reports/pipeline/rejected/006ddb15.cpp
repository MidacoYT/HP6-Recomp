void __fastcall FUN_006ddb15(std::uint32_t* self)
{
    self[0] = reinterpret_cast<std::uint32_t>(&PTR_FUN_0071b5a0);

    // The member at 0x498 has no evidence-backed name.
    auto* field_0498 = reinterpret_cast<void*>(self[0x126]);
    if (field_0498 != nullptr) {
        auto* vtable = *reinterpret_cast<std::uint8_t**>(field_0498);
        auto method = *reinterpret_cast<void (__thiscall **)(void*)>(vtable + 0x20);
        method(field_0498);
    }

    FUN_006de5b0(reinterpret_cast<int>(self));
    FUN_006dd00c(reinterpret_cast<void*>(self[0x119]), reinterpret_cast<int>(self));
    FUN_006de5e1(self);
}
