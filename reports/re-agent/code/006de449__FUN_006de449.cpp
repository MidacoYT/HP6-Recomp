std::uint32_t* __thiscall FUN_006de449(void* self, std::uint8_t param_1)
{
    *reinterpret_cast<void**>(self) = &PTR_FUN_0071b5e8;
    FUN_006de735(self);

    if ((param_1 & 1u) != 0) {
        FUN_006dcbc7(self, 0x464);
    }

    return static_cast<std::uint32_t*>(self);
}