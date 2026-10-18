std::uint32_t* __thiscall FUN_006d85c9(void* self, std::uint32_t param_1)
{
    // The field at offset 0x0c has no evidence-backed member name.
    const auto value = *reinterpret_cast<std::int32_t*>(
        reinterpret_cast<std::uint8_t*>(self) + 0x0c);

    auto* result = reinterpret_cast<std::uint32_t*>(
        FUN_006d6ab6("cl_eagl", value + 0x10, 0x10, 0x10,
                     param_1, "shpclone.cpp", 0x106));

    FUN_006d852e(self, result);
    result[1] = 0;
    result[0] = (result[0] & 0xfffffbffu) | 0x400u;
    return result;
}
