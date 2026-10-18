// The member at +0x0C and the owner type are unresolved in the supplied evidence.
std::uint32_t* __thiscall FUN_006d829c(void* self, std::uint32_t param_1)
{
    const auto value = *reinterpret_cast<std::int32_t*>(
        static_cast<std::byte*>(self) + 0x0C);

    auto* result = reinterpret_cast<std::uint32_t*>(
        FUN_006d6ab6("c_tex", value + 0x20, 0x80, 0x20,
                     param_1, "shpclone.cpp", 0x3C));

    FUN_006d822a(self, result);
    result[1] = 0;
    result[0] = (result[0] & 0xFFFFFBFFu) | 0x400u;
    return result;
}
