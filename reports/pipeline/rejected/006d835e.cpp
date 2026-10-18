std::uint32_t* __thiscall FUN_006d835e(void* self, std::uint32_t param_1)
{
    // The member at +0x0C has no evidence-backed name.
    const std::int32_t value = *reinterpret_cast<std::int32_t*>(
        reinterpret_cast<std::uint8_t*>(self) + 0x0C);

    auto* result = reinterpret_cast<std::uint32_t*>(
        FUN_006d6ab6("cl_clut", value + 0x20, 0x10, 0x20,
                     param_1, "shpclone.cpp", 0x6F));

    FUN_006d82ec(self, result);
    result[1] = 0;
    result[0] = (result[0] & 0xFFFFFBFFu) | 0x400u;
    return result;
}
