std::uint32_t* __thiscall FUN_006d84de(void* self, std::uint32_t param_1)
{
    // The member at offset 0x0c has no evidence-backed name.
    const auto value = *reinterpret_cast<std::uint32_t*>(
        static_cast<std::byte*>(self) + 0x0c);

    auto* result = reinterpret_cast<std::uint32_t*>(
        FUN_006d6ab6("cl_hots", value + 0x20, 0x10, 0x20,
                     param_1, "shpclone.cpp", 0xd2));

    FUN_006d8466(self, result);
    result[1] = 0;
    result[0] = (result[0] & 0xfffffbffu) | 0x400u;
    return result;
}